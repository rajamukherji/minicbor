#include "minicbor.h"
#include <math.h>
#include <stdint.h>

#define EVENT(TYPE) \
	Stream->Available = Available; \
	Stream->Next = Next; \
	return MCE_ ## TYPE

minicbor_event_t MINICBOR(next)(minicbor_stream_t *Stream) {
	unsigned char *Buffer = Stream->Buffer;
	unsigned Available = Stream->Available;
	const unsigned char *Next = Stream->Next;
	for (;;) {
		if (!Available) {
			EVENT(WAIT);
		} else switch (Stream->State) {
		case MCS_DEFAULT: {
			unsigned char Byte = *Next++;
			--Available;
			switch (Byte) {
			case 0x00 ... 0x17:
				Stream->Integer = Byte - 0x00;
				EVENT(POSITIVE);
			case 0x18 ... 0x1B:
				Stream->Size = Stream->Required = 1 << (Byte - 0x18);
				Stream->State = MCS_POSITIVE;
				break;
			case 0x20 ... 0x37:
				Stream->Integer = Byte - 0x20;
				EVENT(NEGATIVE);
			case 0x38 ... 0x3B:
				Stream->Size = Stream->Required = 1 << (Byte - 0x38);
				Stream->State = MCS_NEGATIVE;
				break;
			case 0x40 ... 0x57:
				Stream->Required = Byte - 0x40;
				Stream->State = Stream->Required ? MCS_BYTES : MCS_DEFAULT;
				EVENT(BYTES);
			case 0x58 ... 0x5B:
				Stream->Size = Stream->Required = 1 << (Byte - 0x58);
				Stream->State = MCS_BYTES_SIZE;
				break;
			case 0x5F:
				Stream->State = MCS_BYTES_INDEF;
				Stream->Required = SIZE_MAX;
				EVENT(BYTES);
			case 0x60 ... 0x77:
				Stream->Required = Byte - 0x60;
				Stream->State = Stream->Required ? MCS_STRING : MCS_DEFAULT;
				EVENT(STRING);
			case 0x78 ... 0x7B:
				Stream->Size = Stream->Required = 1 << (Byte - 0x78);
				Stream->State = MCS_STRING_SIZE;
				break;
			case 0x7F:
				Stream->State = MCS_STRING_INDEF;
				Stream->Required = SIZE_MAX;
				EVENT(STRING);
			case 0x80 ... 0x97:
				Stream->Required = Byte = 0x80;
				EVENT(ARRAY);
			case 0x98 ... 0x9B:
				Stream->Size = Stream->Required = 1 << (Byte - 0x98);
				Stream->State = MCS_ARRAY_SIZE;
				break;
			case 0x9F:
				Stream->Required = SIZE_MAX;
				EVENT(ARRAY);
			case 0xA0 ... 0xB7:
				Stream->Required = Byte - 0xA0;
				EVENT(MAP);
			case 0xB8 ... 0xBB:
				Stream->Size = Stream->Required = 1 << (Byte - 0xB8);
				Stream->State = MCS_MAP_SIZE;
				break;
			case 0xBF:
				Stream->Required = SIZE_MAX;
				EVENT(MAP);
			case 0xC0 ... 0xD7:
				Stream->Tag = Byte - 0xC0;
				EVENT(TAG);
			case 0xD8 ... 0xDB:
				Stream->Size = Stream->Required = 1 << (Byte - 0xD8);
				Stream->State = MCS_TAG;
				break;
			case 0xE0 ... 0xF7:
				Stream->Simple = Byte - 0xE0;
				EVENT(SIMPLE);
			case 0xF8:
				Stream->State = MCS_SIMPLE;
				break;
			case 0xF9 ... 0xFB:
				Stream->Size = Stream->Required = 1 << (Byte - 0xF8);
				Stream->State = MCS_FLOAT;
				break;
			case 0xFF:
				EVENT(BREAK);
			}
			break;
		}
		case MCS_POSITIVE: {
			int Required = Stream->Required;
			while (Available && Required) {
				Buffer[--Required] = *Next++;
				--Available;
			}
			if (!Required) {
				uint64_t Integer = 0;
				switch (Stream->Size) {
				case 1: Integer = *(uint8_t *)Buffer; break;
				case 2: Integer = *(uint16_t *)Buffer; break;
				case 4: Integer = *(uint32_t *)Buffer; break;
				case 8: Integer = *(uint64_t *)Buffer; break;
				default: __builtin_unreachable();
				}
				Stream->State = MCS_DEFAULT;
				Stream->Integer = Integer;
				EVENT(POSITIVE);
			} else {
				Stream->Required = Required;
			}
			break;
		}
		case MCS_NEGATIVE: {
			int Required = Stream->Required;
			while (Available && Required) {
				Buffer[--Required] = *Next++;
				--Available;
			}
			if (!Required) {
				uint64_t Integer = 0;
				switch (Stream->Size) {
				case 1: Integer = *(uint8_t *)Buffer; break;
				case 2: Integer = *(uint16_t *)Buffer; break;
				case 4: Integer = *(uint32_t *)Buffer; break;
				case 8: Integer = *(uint64_t *)Buffer; break;
				default: __builtin_unreachable();
				}
				Stream->State = MCS_DEFAULT;
				Stream->Integer = Integer;
				EVENT(NEGATIVE);
			} else {
				Stream->Required = Required;
			}
			break;
		}
		case MCS_BYTES_SIZE: {
			int Required = Stream->Required;
			while (Available && Required) {
				Buffer[--Required] = *Next++;
				--Available;
			}
			if (!Required) {
				int Size = 0;
				switch (Stream->Size) {
				case 1: Size = *(uint8_t *)Buffer; break;
				case 2: Size = *(uint16_t *)Buffer; break;
				case 4: Size = *(uint32_t *)Buffer; break;
				case 8: Size = *(uint64_t *)Buffer; break;
				default: __builtin_unreachable();
				}
				if (Size) {
					Stream->Required = Size;
					Stream->State = MCS_BYTES;
				} else {
					Stream->State = MCS_DEFAULT;
				}
				Stream->Size = Size;
				EVENT(BYTES);
			} else {
				Stream->Required = Required;
			}
			break;
		}
		case MCS_BYTES: {
			int Required = Stream->Required;
			Stream->Bytes = Next;
			if (Available < Required) {
				Stream->Size = Available;
				Stream->Required = Required - Available;
				Next += Available;
				Available = 0;
			} else {
				Stream->Size = Required;
				Stream->Required = 0;
				Next += Required;
				Available -= Required;
				Stream->State = MCS_DEFAULT;
			}
			EVENT(BYTES_PIECE);
		}
		case MCS_BYTES_INDEF: {
			unsigned char Byte = *Next++;
			--Available;
			switch (Byte) {
			case 0x40 ... 0x57:
				Stream->Required = Byte - 0x40;
				Stream->State = MCS_BYTES_CHUNK;
				break;
			case 0x58 ... 0x5B:
				Stream->Required = 1 << (Byte - 0x58);
				Stream->State = MCS_BYTES_CHUNK_SIZE;
				break;
			case 0xFF:
				Stream->State = MCS_DEFAULT;
				Stream->Size = Stream->Required = 0;
				EVENT(BYTES_PIECE);
			default:
				Stream->State = MCS_INVALID;
				EVENT(ERROR);
			}
			break;
		}
		case MCS_BYTES_CHUNK_SIZE: {
			int Required = Stream->Required;
			while (Available && Required) {
				Buffer[--Required] = *Next++;
				--Available;
			}
			if (!Required) {
				int Size = 0;
				switch (Stream->Size) {
				case 1: Size = *(uint8_t *)Buffer; break;
				case 2: Size = *(uint16_t *)Buffer; break;
				case 4: Size = *(uint32_t *)Buffer; break;
				case 8: Size = *(uint64_t *)Buffer; break;
				default: __builtin_unreachable();
				}
				Stream->Required = Size;
				Stream->State = MCS_BYTES_CHUNK;
			} else {
				Stream->Required = Required;
			}
			break;
		}
		case MCS_BYTES_CHUNK: {
			int Required = Stream->Required;
			if (Available < Required) {
				Stream->Size = Available;
				Stream->Required = Required - Available;
				Next += Available;
				Available = 0;
			} else {
				Stream->Size = Required;
				Stream->Required = 0;
				Next += Required;
				Available -= Required;
				Stream->State = MCS_BYTES_INDEF;
			}
			EVENT(BYTES_PIECE);
		}
		case MCS_STRING_SIZE: {
			int Required = Stream->Required;
			while (Available && Required) {
				Buffer[--Required] = *Next++;
				--Available;
			}
			if (!Required) {
				int Size = 0;
				switch (Stream->Size) {
				case 1: Size = *(uint8_t *)Buffer; break;
				case 2: Size = *(uint16_t *)Buffer; break;
				case 4: Size = *(uint32_t *)Buffer; break;
				case 8: Size = *(uint64_t *)Buffer; break;
				default: __builtin_unreachable();
				}
				if (Size) {
					Stream->Required = Size;
					Stream->State = MCS_STRING;
				} else {
					Stream->State = MCS_DEFAULT;
				}
				Stream->Size = Size;
				EVENT(STRING);
			} else {
				Stream->Required = Required;
			}
			break;
		}
		case MCS_STRING: {
			int Required = Stream->Required;
			Stream->Bytes = Next;
			if (Available < Required) {
				Stream->Size = Available;
				Stream->Required = Required - Available;
				Next += Available;
				Available = 0;
			} else {
				Stream->Size = Required;
				Stream->Required = 0;
				Next += Required;
				Available -= Required;
				Stream->State = MCS_DEFAULT;
			}
			EVENT(STRING_PIECE);
		}
		case MCS_STRING_INDEF: {
			unsigned char Byte = *Next++;
			--Available;
			switch (Byte) {
			case 0x60 ... 0x77:
				Stream->Required = Byte - 0x60;
				Stream->State = MCS_STRING_CHUNK;
				break;
			case 0x78 ... 0x7B:
				Stream->Required = 1 << (Byte - 0x78);
				Stream->State = MCS_STRING_CHUNK_SIZE;
				break;
			case 0xFF:
				Stream->State = MCS_DEFAULT;
				Stream->Size = Stream->Required = 0;
				EVENT(STRING_PIECE);
			default:
				Stream->State = MCS_INVALID;
				EVENT(ERROR);
			}
			break;
		}
		case MCS_STRING_CHUNK_SIZE: {
			int Required = Stream->Required;
			while (Available && Required) {
				Buffer[--Required] = *Next++;
				--Available;
			}
			if (!Required) {
				int Size = 0;
				switch (Stream->Size) {
				case 1: Size = *(uint8_t *)Buffer; break;
				case 2: Size = *(uint16_t *)Buffer; break;
				case 4: Size = *(uint32_t *)Buffer; break;
				case 8: Size = *(uint64_t *)Buffer; break;
				default: __builtin_unreachable();
				}
				Stream->Required = Size;
				Stream->State = MCS_STRING_CHUNK;
			} else {
				Stream->Required = Required;
			}
			break;
		}
		case MCS_STRING_CHUNK: {
			int Required = Stream->Required;
			if (Available < Required) {
				Stream->Size = Available;
				Stream->Required = Required - Available;
				Next += Available;
				Available = 0;
			} else {
				Stream->Size = Required;
				Stream->Required = 0;
				Next += Required;
				Available -= Required;
				Stream->State = MCS_BYTES_INDEF;
			}
			EVENT(STRING_PIECE);
		}
		case MCS_ARRAY_SIZE: {
			int Required = Stream->Required;
			while (Available && Required) {
				Buffer[--Required] = *Next++;
				--Available;
			}
			if (!Required) {
				int Size = 0;
				switch (Stream->Size) {
				case 1: Size = *(uint8_t *)Buffer; break;
				case 2: Size = *(uint16_t *)Buffer; break;
				case 4: Size = *(uint32_t *)Buffer; break;
				case 8: Size = *(uint64_t *)Buffer; break;
				default: __builtin_unreachable();
				}
				Stream->Required = Size;
				Stream->State = MCS_DEFAULT;
				EVENT(ARRAY);
			} else {
				Stream->Required = Required;
			}
			break;
		}
		case MCS_MAP_SIZE: {
			int Required = Stream->Required;
			while (Available && Required) {
				Buffer[--Required] = *Next++;
				--Available;
			}
			if (!Required) {
				int Size = 0;
				switch (Stream->Size) {
				case 1: Size = *(uint8_t *)Buffer; break;
				case 2: Size = *(uint16_t *)Buffer; break;
				case 4: Size = *(uint32_t *)Buffer; break;
				case 8: Size = *(uint64_t *)Buffer; break;
				default: __builtin_unreachable();
				}
				Stream->Required = Size;
				Stream->State = MCS_DEFAULT;
				EVENT(MAP);
			} else {
				Stream->Required = Required;
			}
			break;
		}
		case MCS_TAG: {
			int Required = Stream->Required;
			while (Available && Required) {
				Buffer[--Required] = *Next++;
				--Available;
			}
			if (!Required) {
				uint64_t Tag = 0;
				switch (Stream->Size) {
				case 1: Tag = *(uint8_t *)Buffer; break;
				case 2: Tag = *(uint16_t *)Buffer; break;
				case 4: Tag = *(uint32_t *)Buffer; break;
				case 8: Tag = *(uint64_t *)Buffer; break;
				default: __builtin_unreachable();
				}
				Stream->Tag = Tag;
				Stream->State = MCS_DEFAULT;
				EVENT(TAG);
			} else {
				Stream->Required = Required;
			}
			break;
		}
		case MCS_SIMPLE: {
			int Value = *Next++;
			--Available;
			Stream->Simple = Value;
			Stream->State = MCS_DEFAULT;
			EVENT(SIMPLE);
		}
		case MCS_FLOAT: {
			int Required = Stream->Required;
			while (Available && Required) {
				Buffer[--Required] = *Next++;
				--Available;
			}
			if (!Required) {
				double Real = 0;
				switch (Stream->Size) {
				case 2: {
					int Half = (Buffer[1] << 8) + Buffer[0];
					int Exp = (Half >> 10) & 0x1F;
					int Mant = Half & 0x3FF;
					if (Exp == 0) {
						Real = ldexp(Mant, -24);
					} else if (Exp != 31) {
						Real = ldexp(Mant + 1024, Exp - 25);
					} else {
						Real = Mant ? NAN : INFINITY;
					}
					if (Half & 0x8000) Real = -Real;
					break;
				}
				case 4: Real = *(float *)Buffer; break;
				case 8: Real = *(double *)Buffer; break;
				default: __builtin_unreachable();
				}
				Stream->Real = Real;
				Stream->State = MCS_DEFAULT;
				EVENT(FLOAT);
			} else {
				Stream->Required = Required;
			}
			break;
		}
		case MCS_INVALID:
		case MCS_FINISHED:
			EVENT(ERROR);
		default: __builtin_unreachable();
		}
	}
	EVENT(ERROR);
}

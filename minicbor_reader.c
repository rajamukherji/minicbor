#include "minicbor.h"
#include <math.h>

void MINICBOR(reader_init)(minicbor_reader_t *Reader) {
	Reader->Position = 0;
	Reader->State = MCS_DEFAULT;
}

#ifdef MINICBOR_READ_FN_PREFIX

#define POSITIVE_FN MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, positive_fn)
#define NEGATIVE_FN MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, negative_fn)
#define BYTES_FN MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, bytes_fn)
#define BYTES_PIECE_FN MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, bytes_piece_fn)
#define STRING_FN MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, string_fn)
#define STRING_PIECE_FN MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, string_piece_fn)
#define ARRAY_FN MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, array_fn)
#define MAP_FN MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, map_fn)
#define TAG_FN MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, tag_fn)
#define SIMPLE_FN MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, simple_fn)
#define FLOAT_FN MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, float_fn)
#define BREAK_FN MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, break_fn)
#define ERROR_FN MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, error_fn)

#else

#define POSITIVE_FN Reader->Callbacks->PositiveFn
#define NEGATIVE_FN Reader->Callbacks->NegativeFn
#define BYTES_FN Reader->Callbacks->BytesFn
#define BYTES_PIECE_FN Reader->Callbacks->BytesPieceFn
#define STRING_FN Reader->Callbacks->StringFn
#define STRING_PIECE_FN Reader->Callbacks->StringPieceFn
#define ARRAY_FN Reader->Callbacks->ArrayFn
#define MAP_FN Reader->Callbacks->MapFn
#define TAG_FN Reader->Callbacks->TagFn
#define SIMPLE_FN Reader->Callbacks->SimpleFn
#define FLOAT_FN Reader->Callbacks->FloatFn
#define BREAK_FN Reader->Callbacks->BreakFn
#define ERROR_FN Reader->Callbacks->ErrorFn

#endif

int MINICBOR(read)(minicbor_reader_t *Reader, const unsigned char *Bytes, unsigned Available) {
	minicbor_buffer_t *Buffer = Reader->Buffer;
	Reader->Position += Available;
	for (;;) {
		if (Reader->State == MCS_FINISHED) {
			Reader->Required = Available;
			return 1;
		} else if (!Available) {
			return 0;
		} else switch (Reader->State) {
		case MCS_DEFAULT: {
			unsigned char Byte = *Bytes++;
			--Available;
			switch (Byte) {
			case 0x00 ... 0x17:
				POSITIVE_FN(Reader->UserData, Byte - 0x00);
				break;
			case 0x18 ... 0x1B:
				Reader->Width = Reader->Required = 1 << (Byte - 0x18);
				Reader->State = MCS_POSITIVE;
				break;
			case 0x20 ... 0x37:
				NEGATIVE_FN(Reader->UserData, Byte - 0x20);
				break;
			case 0x38 ... 0x3B:
				Reader->Width = Reader->Required = 1 << (Byte - 0x38);
				Reader->State = MCS_NEGATIVE;
				break;
			case 0x40 ... 0x57:
				Reader->Required = Byte - 0x40;
				Reader->State = Reader->Required ? MCS_BYTES : MCS_DEFAULT;
				BYTES_FN(Reader->UserData, Reader->Required);
				break;
			case 0x58 ... 0x5B:
				Reader->Width = Reader->Required = 1 << (Byte - 0x58);
				Reader->State = MCS_BYTES_SIZE;
				break;
			case 0x5F:
				Reader->State = MCS_BYTES_INDEF;
				BYTES_FN(Reader->UserData, -1);
				break;
			case 0x60 ... 0x77:
				Reader->Required = Byte - 0x60;
				Reader->State = Reader->Required ? MCS_STRING : MCS_DEFAULT;
				STRING_FN(Reader->UserData, Reader->Required);
				break;
			case 0x78 ... 0x7B:
				Reader->Width = Reader->Required = 1 << (Byte - 0x78);
				Reader->State = MCS_STRING_SIZE;
				break;
			case 0x7F:
				Reader->State = MCS_STRING_INDEF;
				STRING_FN(Reader->UserData, -1);
				break;
			case 0x80 ... 0x97:
				ARRAY_FN(Reader->UserData, Byte - 0x80);
				break;
			case 0x98 ... 0x9B:
				Reader->Width = Reader->Required = 1 << (Byte - 0x98);
				Reader->State = MCS_ARRAY_SIZE;
				break;
			case 0x9F:
				ARRAY_FN(Reader->UserData, -1);
				break;
			case 0xA0 ... 0xB7:
				MAP_FN(Reader->UserData, Byte - 0xA0);
				break;
			case 0xB8 ... 0xBB:
				Reader->Width = Reader->Required = 1 << (Byte - 0xB8);
				Reader->State = MCS_MAP_SIZE;
				break;
			case 0xBF:
				MAP_FN(Reader->UserData, -1);
				break;
			case 0xC0 ... 0xD7:
				TAG_FN(Reader->UserData, Byte - 0xC0);
				break;
			case 0xD8 ... 0xDB:
				Reader->Width = Reader->Required = 1 << (Byte - 0xD8);
				Reader->State = MCS_TAG;
				break;
			case 0xE0 ... 0xF7:
				SIMPLE_FN(Reader->UserData, Byte - 0xE0);
				break;
			case 0xF8:
				Reader->State = MCS_SIMPLE;
				break;
			case 0xF9 ... 0xFB:
				Reader->Width = Reader->Required = 1 << (Byte - 0xF8);
				Reader->State = MCS_FLOAT;
				break;
			case 0xFF:
				BREAK_FN(Reader->UserData);
				break;
			}
			break;
		}
		case MCS_POSITIVE: {
			int Required = Reader->Required;
			while (Available && Required) {
				Buffer->Bytes[--Required] = *Bytes++;
				--Available;
			}
			if (!Required) {
				uint64_t Number = 0;
				switch (Reader->Width) {
				case 1: Number = Buffer->Bytes[0]; break;
				case 2: Number = Buffer->Int16; break;
				case 4: Number = Buffer->Int32; break;
				case 8: Number = Buffer->Int64; break;
				}
				Reader->State = MCS_DEFAULT;
				POSITIVE_FN(Reader->UserData, Number);
			} else {
				Reader->Required = Required;
			}
			break;
		}
		case MCS_NEGATIVE: {
			int Required = Reader->Required;
			while (Available && Required) {
				Buffer->Bytes[--Required] = *Bytes++;
				--Available;
			}
			if (!Required) {
				uint64_t Number = 0;
				switch (Reader->Width) {
				case 1: Number = Buffer->Bytes[0]; break;
				case 2: Number = Buffer->Int16; break;
				case 4: Number = Buffer->Int32; break;
				case 8: Number = Buffer->Int64; break;
				}
				Reader->State = MCS_DEFAULT;
				NEGATIVE_FN(Reader->UserData, Number);
			} else {
				Reader->Required = Required;
			}
			break;
		}
		case MCS_BYTES_SIZE: {
			int Required = Reader->Required;
			while (Available && Required) {
				Buffer->Bytes[--Required] = *Bytes++;
				--Available;
			}
			if (!Required) {
				int Size = 0;
				switch (Reader->Width) {
				case 1: Size = Buffer->Bytes[0]; break;
				case 2: Size = Buffer->Int16; break;
				case 4: Size = Buffer->Int32; break;
				case 8: Size = Buffer->Int64; break;
				}
				if (Size) {
					Reader->Required = Size;
					Reader->State = MCS_BYTES;
				} else {
					Reader->State = MCS_DEFAULT;
				}
				BYTES_FN(Reader->UserData, Size);
			} else {
				Reader->Required = Required;
			}
			break;
		}
		case MCS_BYTES: {
			int Required = Reader->Required;
			if (Available < Required) {
				BYTES_PIECE_FN(Reader->UserData, Bytes, Available, 0);
				Reader->Required = Required - Available;
				Available = 0;
			} else {
				Reader->State = MCS_DEFAULT;
				BYTES_PIECE_FN(Reader->UserData, Bytes, Required, 1);
				Available -= Required;
				Bytes += Required;
			}
			break;
		}
		case MCS_BYTES_INDEF: {
			unsigned char Byte = *Bytes++;
			--Available;
			switch (Byte) {
			case 0x40 ... 0x57:
				Reader->Required = Byte - 0x40;
				Reader->State = MCS_BYTES_CHUNK;
				break;
			case 0x58 ... 0x5B:
				Reader->Required = 1 << (Byte - 0x58);
				Reader->State = MCS_BYTES_CHUNK_SIZE;
				break;
			case 0xFF:
				Reader->State = MCS_DEFAULT;
				BYTES_PIECE_FN(Reader->UserData, Bytes, 0, 1);
				break;
			default:
				Reader->State = MCS_INVALID;
				ERROR_FN(Reader->UserData, Reader->Position - Available, "Invalid content in indefinite bytestring");
				break;
			}
			break;
		}
		case MCS_BYTES_CHUNK_SIZE: {
			int Required = Reader->Required;
			while (Available && Required) {
				Buffer->Bytes[--Required] = *Bytes++;
				--Available;
			}
			if (!Required) {
				int Size = 0;
				switch (Reader->Width) {
				case 1: Size = Buffer->Bytes[0]; break;
				case 2: Size = Buffer->Int16; break;
				case 4: Size = Buffer->Int32; break;
				case 8: Size = Buffer->Int64; break;
				}
				Reader->Required = Size;
				Reader->State = MCS_BYTES_CHUNK;
			} else {
				Reader->Required = Required;
			}
			break;
		}
		case MCS_BYTES_CHUNK: {
			int Required = Reader->Required;
			if (Available < Required) {
				BYTES_PIECE_FN(Reader->UserData, Bytes, Available, 0);
				Reader->Required = Required - Available;
				Available = 0;
			} else {
				Reader->State = MCS_BYTES_INDEF;
				BYTES_PIECE_FN(Reader->UserData, Bytes, Required, 0);
				Available -= Required;
				Bytes += Required;
			}
			break;
		}
		case MCS_STRING_SIZE: {
			int Required = Reader->Required;
			while (Available && Required) {
				Buffer->Bytes[--Required] = *Bytes++;
				--Available;
			}
			if (!Required) {
				int Size = 0;
				switch (Reader->Width) {
				case 1: Size = Buffer->Bytes[0]; break;
				case 2: Size = Buffer->Int16; break;
				case 4: Size = Buffer->Int32; break;
				case 8: Size = Buffer->Int64; break;
				}
				if (Size) {
					Reader->Required = Size;
					Reader->State = MCS_STRING;
				} else {
					Reader->State = MCS_DEFAULT;
				}
				STRING_FN(Reader->UserData, Size);
			} else {
				Reader->Required = Required;
			}
			break;
		}
		case MCS_STRING: {
			int Required = Reader->Required;
			if (Available < Required) {
				STRING_PIECE_FN(Reader->UserData, Bytes, Available, 0);
				Reader->Required = Required - Available;
				Available = 0;
			} else {
				Reader->State = MCS_DEFAULT;
				STRING_PIECE_FN(Reader->UserData, Bytes, Required, 1);
				Available -= Required;
				Bytes += Required;
			}
			break;
		}
		case MCS_STRING_INDEF: {
			unsigned char Byte = *Bytes++;
			--Available;
			switch (Byte) {
			case 0x60 ... 0x77:
				Reader->Required = Byte - 0x60;
				Reader->State = MCS_STRING_CHUNK;
				break;
			case 0x78 ... 0x7B:
				Reader->Required = 1 << (Byte - 0x78);
				Reader->State = MCS_STRING_CHUNK_SIZE;
				break;
			case 0xFF:
				Reader->State = MCS_DEFAULT;
				STRING_PIECE_FN(Reader->UserData, Bytes, 0, 1);
				break;
			default:
				Reader->State = MCS_INVALID;
				ERROR_FN(Reader->UserData, Reader->Position - Available, "Invalid content in indefinite string");
				break;
			}
			break;
		}
		case MCS_STRING_CHUNK_SIZE: {
			int Required = Reader->Required;
			while (Available && Required) {
				Buffer->Bytes[--Required] = *Bytes++;
				--Available;
			}
			if (!Required) {
				int Size = 0;
				switch (Reader->Width) {
				case 1: Size = Buffer->Bytes[0]; break;
				case 2: Size = Buffer->Int16; break;
				case 4: Size = Buffer->Int32; break;
				case 8: Size = Buffer->Int64; break;
				}
				Reader->Required = Size;
				Reader->State = MCS_STRING_CHUNK;
			} else {
				Reader->Required = Required;
			}
			break;
		}
		case MCS_STRING_CHUNK: {
			int Required = Reader->Required;
			if (Available < Required) {
				STRING_PIECE_FN(Reader->UserData, Bytes, Available, 0);
				Reader->Required = Required - Available;
				Available = 0;
			} else {
				Reader->State = MCS_STRING_INDEF;
				STRING_PIECE_FN(Reader->UserData, Bytes, Required, 0);
				Available -= Required;
				Bytes += Required;
			}
			break;
		}
		case MCS_ARRAY_SIZE: {
			int Required = Reader->Required;
			while (Available && Required) {
				Buffer->Bytes[--Required] = *Bytes++;
				--Available;
			}
			if (!Required) {
				int Size = 0;
				switch (Reader->Width) {
				case 1: Size = Buffer->Bytes[0]; break;
				case 2: Size = Buffer->Int16; break;
				case 4: Size = Buffer->Int32; break;
				case 8: Size = Buffer->Int64; break;
				}
				Reader->Required = Size;
				Reader->State = MCS_DEFAULT;
				ARRAY_FN(Reader->UserData, Size);
			} else {
				Reader->Required = Required;
			}
			break;
		}
		case MCS_MAP_SIZE: {
			int Required = Reader->Required;
			while (Available && Required) {
				Buffer->Bytes[--Required] = *Bytes++;
				--Available;
			}
			if (!Required) {
				int Size = 0;
				switch (Reader->Width) {
				case 1: Size = Buffer->Bytes[0]; break;
				case 2: Size = Buffer->Int16; break;
				case 4: Size = Buffer->Int32; break;
				case 8: Size = Buffer->Int64; break;
				}
				Reader->Required = Size;
				Reader->State = MCS_DEFAULT;
				MAP_FN(Reader->UserData, Size);
			} else {
				Reader->Required = Required;
			}
			break;
		}
		case MCS_TAG: {
			int Required = Reader->Required;
			while (Available && Required) {
				Buffer->Bytes[--Required] = *Bytes++;
				--Available;
			}
			if (!Required) {
				uint64_t Tag = 0;
				switch (Reader->Width) {
				case 1: Tag = Buffer->Bytes[0]; break;
				case 2: Tag = Buffer->Int16; break;
				case 4: Tag = Buffer->Int32; break;
				case 8: Tag = Buffer->Int64; break;
				}
				Reader->State = MCS_DEFAULT;
				TAG_FN(Reader->UserData, Tag);
			} else {
				Reader->Required = Required;
			}
			break;
		}
		case MCS_SIMPLE: {
			int Value = *Bytes++;
			--Available;
			Reader->State = MCS_DEFAULT;
			SIMPLE_FN(Reader->UserData, Value);
			break;
		}
		case MCS_FLOAT: {
			int Required = Reader->Required;
			while (Available && Required) {
				Buffer->Bytes[--Required] = *Bytes++;
				--Available;
			}
			if (!Required) {
				double Number = 0;
				switch (Reader->Width) {
				case 2: {
					int Half = (Buffer->Bytes[1] << 8) + Buffer->Bytes[0];
					int Exp = (Half >> 10) & 0x1F;
					int Mant = Half & 0x3FF;
					if (Exp == 0) {
						Number = ldexp(Mant, -24);
					} else if (Exp != 31) {
						Number = ldexp(Mant + 1024, Exp - 25);
					} else {
						Number = Mant ? NAN : INFINITY;
					}
					if (Half & 0x8000) Number = -Number;
					break;
				}
				case 4: Number = Buffer->Float; break;
				case 8: Number = Buffer->Double; break;
				}
				Reader->State = MCS_DEFAULT;
				FLOAT_FN(Reader->UserData, Number);
			} else {
				Reader->Required = Required;
			}
			break;
		}
		case MCS_INVALID:
			ERROR_FN(Reader->UserData, Reader->Position - Available, "Reader in invalid state");
			return 1;
		case MCS_FINISHED:
			Reader->Required = Available;
			return 1;
		}
	}
	return 0;
}

void MINICBOR(reader_finish)(minicbor_reader_t *Reader) {
	Reader->State = MCS_FINISHED;
}

int MINICBOR(reader_remaining)(minicbor_reader_t *Reader) {
	return Reader->Required;
}

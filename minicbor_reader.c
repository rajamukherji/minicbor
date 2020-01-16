#include "minicbor.h"
#include <math.h>

void MINICBOR(reader_init)(minicbor_reader_t *Reader) {
	Reader->Position = 0;
	Reader->State = MCS_DEFAULT;
}

static inline uint64_t MINICBOR(read8)(unsigned char *Bytes) {
	return *(uint8_t *)Bytes;
}

static inline uint64_t MINICBOR(read16)(unsigned char *Bytes) {
	return *(uint16_t *)Bytes;;
}

static inline uint64_t MINICBOR(read32)(unsigned char *Bytes) {
	return *(uint32_t *)Bytes;
}

static inline uint64_t MINICBOR(read64)(unsigned char *Bytes) {
	return *(uint64_t *)Bytes;
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

#define POSITIVE_FN Reader->PositiveFn
#define NEGATIVE_FN Reader->NegativeFn
#define BYTES_FN Reader->BytesFn
#define BYTES_PIECE_FN Reader->BytesPieceFn
#define STRING_FN Reader->StringFn
#define STRING_PIECE_FN Reader->StringPieceFn
#define ARRAY_FN Reader->ArrayFn
#define MAP_FN Reader->MapFn
#define TAG_FN Reader->TagFn
#define SIMPLE_FN Reader->SimpleFn
#define FLOAT_FN Reader->FloatFn
#define BREAK_FN Reader->BreakFn
#define ERROR_FN Reader->ErrorFn

#endif

int MINICBOR(read)(minicbor_reader_t *Reader, const unsigned char *Bytes, unsigned Available) {
	unsigned char *Buffer = Reader->Buffer;
	Reader->Position += Available;
	minicbor_state_t State = Reader->State;
	while (Available) switch (State) {
	case MCS_DEFAULT: {
		unsigned char Byte = *Bytes++;
		--Available;
		switch (Byte) {
		case 0x00 ... 0x17:
			POSITIVE_FN(Reader->UserData, Byte - 0x00);
			break;
		case 0x18 ... 0x1B:
			Reader->Width = Reader->Required = 1 << (Byte - 0x18);
			State = MCS_POSITIVE;
			break;
		case 0x20 ... 0x37:
			NEGATIVE_FN(Reader->UserData, Byte - 0x20);
			break;
		case 0x38 ... 0x3B:
			Reader->Width = Reader->Required = 1 << (Byte - 0x38);
			State = MCS_NEGATIVE;
			break;
		case 0x40 ... 0x57:
			Reader->Required = Byte - 0x40;
			BYTES_FN(Reader->UserData, Reader->Required);
			State = Reader->Required ? MCS_BYTES : MCS_DEFAULT;
			break;
		case 0x58 ... 0x5B:
			Reader->Width = Reader->Required = 1 << (Byte - 0x58);
			State = MCS_BYTES_SIZE;
			break;
		case 0x5F:
			BYTES_FN(Reader->UserData, -1);
			State = MCS_BYTES_INDEF;
			break;
		case 0x60 ... 0x77:
			Reader->Required = Byte - 0x60;
			STRING_FN(Reader->UserData, Reader->Required);
			State = Reader->Required ? MCS_STRING : MCS_DEFAULT;
			break;
		case 0x78 ... 0x7B:
			Reader->Width = Reader->Required = 1 << (Byte - 0x78);
			State = MCS_STRING_SIZE;
			break;
		case 0x7F:
			STRING_FN(Reader->UserData, -1);
			State = MCS_STRING_INDEF;
			break;
		case 0x80 ... 0x97:
			ARRAY_FN(Reader->UserData, Byte - 0x80);
			break;
		case 0x98 ... 0x9B:
			Reader->Width = Reader->Required = 1 << (Byte - 0x98);
			State = MCS_ARRAY_SIZE;
			break;
		case 0x9F:
			ARRAY_FN(Reader->UserData, -1);
			break;
		case 0xA0 ... 0xB7:
			MAP_FN(Reader->UserData, Byte - 0xA0);
			break;
		case 0xB8 ... 0xBB:
			Reader->Width = Reader->Required = 1 << (Byte - 0xB8);
			State = MCS_MAP_SIZE;
			break;
		case 0xBF:
			MAP_FN(Reader->UserData, -1);
			break;
		case 0xC0 ... 0xD7:
			TAG_FN(Reader->UserData, Byte - 0xC0);
			break;
		case 0xD8 ... 0xDB:
			Reader->Width = Reader->Required = 1 << (Byte - 0xD8);
			State = MCS_TAG;
			break;
		case 0xE0 ... 0xF7:
			SIMPLE_FN(Reader->UserData, Byte - 0xE0);
			break;
		case 0xF8:
			State = MCS_SIMPLE;
			break;
		case 0xF9 ... 0xFB:
			Reader->Width = Reader->Required = 1 << (Byte - 0xF8);
			State = MCS_FLOAT;
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
			Buffer[--Required] = *Bytes++;
			--Available;
		}
		if (!Required) {
			uint64_t Number = 0;
			switch (Reader->Width) {
			case 1: Number = MINICBOR(read8)(Buffer); break;
			case 2: Number = MINICBOR(read16)(Buffer); break;
			case 4: Number = MINICBOR(read32)(Buffer); break;
			case 8: Number = MINICBOR(read64)(Buffer); break;
			}
			POSITIVE_FN(Reader->UserData, Number);
			State = MCS_DEFAULT;
		} else {
			Reader->Required = Required;
		}
		break;
	}
	case MCS_NEGATIVE: {
		int Required = Reader->Required;
		while (Available && Required) {
			Buffer[--Required] = *Bytes++;
			--Available;
		}
		if (!Required) {
			uint64_t Number = 0;
			switch (Reader->Width) {
			case 1: Number = MINICBOR(read8)(Buffer); break;
			case 2: Number = MINICBOR(read16)(Buffer); break;
			case 4: Number = MINICBOR(read32)(Buffer); break;
			case 8: Number = MINICBOR(read64)(Buffer); break;
			}
			NEGATIVE_FN(Reader->UserData, Number);
			State = MCS_DEFAULT;
		} else {
			Reader->Required = Required;
		}
		break;
	}
	case MCS_BYTES_SIZE: {
		int Required = Reader->Required;
		while (Available && Required) {
			Buffer[--Required] = *Bytes++;
			--Available;
		}
		if (!Required) {
			int Size = 0;
			switch (Reader->Width) {
			case 1: Size = MINICBOR(read8)(Buffer); break;
			case 2: Size = MINICBOR(read16)(Buffer); break;
			case 4: Size = MINICBOR(read32)(Buffer); break;
			case 8: Size = MINICBOR(read64)(Buffer); break;
			}
			BYTES_FN(Reader->UserData, Size);
			if (Size) {
				Reader->Required = Size;
				State = MCS_BYTES;
			} else {
				State = MCS_DEFAULT;
			}
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
			BYTES_PIECE_FN(Reader->UserData, Bytes, Required, 1);
			State = MCS_DEFAULT;
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
			State = MCS_BYTES_CHUNK;
			break;
		case 0x58 ... 0x5B:
			Reader->Required = 1 << (Byte - 0x58);
			State = MCS_BYTES_CHUNK_SIZE;
			break;
		case 0xFF:
			BYTES_PIECE_FN(Reader->UserData, Bytes, 0, 1);
			State = MCS_DEFAULT;
			break;
		default:
			ERROR_FN(Reader->UserData, Reader->Position - Available, "Invalid content in indefinite bytestring");
			State = MCS_INVALID;
			break;
		}
		break;
	}
	case MCS_BYTES_CHUNK_SIZE: {
		int Required = Reader->Required;
		while (Available && Required) {
			Buffer[--Required] = *Bytes++;
			--Available;
		}
		if (!Required) {
			int Size = 0;
			switch (Reader->Width) {
			case 1: Size = MINICBOR(read8)(Buffer); break;
			case 2: Size = MINICBOR(read16)(Buffer); break;
			case 4: Size = MINICBOR(read32)(Buffer); break;
			case 8: Size = MINICBOR(read64)(Buffer); break;
			}
			Reader->Required = Size;
			State = MCS_BYTES_CHUNK;
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
			BYTES_PIECE_FN(Reader->UserData, Bytes, Required, 0);
			State = MCS_BYTES_INDEF;
			Available -= Required;
			Bytes += Required;
		}
		break;
	}
	case MCS_STRING_SIZE: {
		int Required = Reader->Required;
		while (Available && Required) {
			Buffer[--Required] = *Bytes++;
			--Available;
		}
		if (!Required) {
			int Size = 0;
			switch (Reader->Width) {
			case 1: Size = MINICBOR(read8)(Buffer); break;
			case 2: Size = MINICBOR(read16)(Buffer); break;
			case 4: Size = MINICBOR(read32)(Buffer); break;
			case 8: Size = MINICBOR(read64)(Buffer); break;
			}
			STRING_FN(Reader->UserData, Size);
			if (Size) {
				Reader->Required = Size;
				State = MCS_STRING;
			} else {
				State = MCS_DEFAULT;
			}
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
			STRING_PIECE_FN(Reader->UserData, Bytes, Required, 1);
			State = MCS_DEFAULT;
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
			State = MCS_STRING_CHUNK;
			break;
		case 0x78 ... 0x7B:
			Reader->Required = 1 << (Byte - 0x78);
			State = MCS_STRING_CHUNK_SIZE;
			break;
		case 0xFF:
			STRING_PIECE_FN(Reader->UserData, Bytes, 0, 1);
			State = MCS_DEFAULT;
			break;
		default:
			ERROR_FN(Reader->UserData, Reader->Position - Available, "Invalid content in indefinite string");
			State = MCS_INVALID;
			break;
		}
		break;
	}
	case MCS_STRING_CHUNK_SIZE: {
		int Required = Reader->Required;
		while (Available && Required) {
			Buffer[--Required] = *Bytes++;
			--Available;
		}
		if (!Required) {
			int Size = 0;
			switch (Reader->Width) {
			case 1: Size = MINICBOR(read8)(Buffer); break;
			case 2: Size = MINICBOR(read16)(Buffer); break;
			case 4: Size = MINICBOR(read32)(Buffer); break;
			case 8: Size = MINICBOR(read64)(Buffer); break;
			}
			Reader->Required = Size;
			State = MCS_STRING_CHUNK;
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
			STRING_PIECE_FN(Reader->UserData, Bytes, Required, 0);
			State = MCS_STRING_INDEF;
			Available -= Required;
			Bytes += Required;
		}
		break;
	}
	case MCS_ARRAY_SIZE: {
		int Required = Reader->Required;
		while (Available && Required) {
			Buffer[--Required] = *Bytes++;
			--Available;
		}
		if (!Required) {
			int Size = 0;
			switch (Reader->Width) {
			case 1: Size = MINICBOR(read8)(Buffer); break;
			case 2: Size = MINICBOR(read16)(Buffer); break;
			case 4: Size = MINICBOR(read32)(Buffer); break;
			case 8: Size = MINICBOR(read64)(Buffer); break;
			}
			ARRAY_FN(Reader->UserData, Size);
			Reader->Required = Size;
			State = MCS_DEFAULT;
		} else {
			Reader->Required = Required;
		}
		break;
	}
	case MCS_MAP_SIZE: {
		int Required = Reader->Required;
		while (Available && Required) {
			Buffer[--Required] = *Bytes++;
			--Available;
		}
		if (!Required) {
			int Size = 0;
			switch (Reader->Width) {
			case 1: Size = MINICBOR(read8)(Buffer); break;
			case 2: Size = MINICBOR(read16)(Buffer); break;
			case 4: Size = MINICBOR(read32)(Buffer); break;
			case 8: Size = MINICBOR(read64)(Buffer); break;
			}
			MAP_FN(Reader->UserData, Size);
			Reader->Required = Size;
			State = MCS_DEFAULT;
		} else {
			Reader->Required = Required;
		}
		break;
	}
	case MCS_TAG: {
		int Required = Reader->Required;
		while (Available && Required) {
			Buffer[--Required] = *Bytes++;
			--Available;
		}
		if (!Required) {
			uint64_t Tag = 0;
			switch (Reader->Width) {
			case 1: Tag = MINICBOR(read8)(Buffer); break;
			case 2: Tag = MINICBOR(read16)(Buffer); break;
			case 4: Tag = MINICBOR(read32)(Buffer); break;
			case 8: Tag = MINICBOR(read64)(Buffer); break;
			}
			TAG_FN(Reader->UserData, Tag);
			State = MCS_DEFAULT;
		} else {
			Reader->Required = Required;
		}
		break;
	}
	case MCS_SIMPLE: {
		int Value = *Bytes++;
		--Available;
		SIMPLE_FN(Reader->UserData, Value);
		State = MCS_DEFAULT;
		break;
	}
	case MCS_FLOAT: {
		int Required = Reader->Required;
		while (Available && Required) {
			Buffer[--Required] = *Bytes++;
			--Available;
		}
		if (!Required) {
			double Number = 0;
			switch (Reader->Width) {
			case 2: {
				int Half = (Buffer[1] << 8) + Buffer[0];
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
			case 4: Number = *(float *)Buffer; break;
			case 8: Number = *(double *)Buffer; break;
			}
			FLOAT_FN(Reader->UserData, Number);
			State = MCS_DEFAULT;
		} else {
			Reader->Required = Required;
		}
		break;
	}
	case MCS_INVALID:
		ERROR_FN(Reader->UserData, Reader->Position - Available, "Reader in invalid state");
		break;
	case MCS_FINISHED:
		Reader->Required = Available;
		return 1;
	}
	Reader->State = State;
	return 0;
}

void MINICBOR(reader_finish)(minicbor_reader_t *Reader) {
	Reader->State = MCS_FINISHED;
}

int MINICBOR(reader_remaining)(minicbor_reader_t *Reader) {
	return Reader->Required;
}

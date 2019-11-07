#include "minicbor.h"

#include <math.h>

typedef enum {
	MCS_DEFAULT,
	MCS_POSITIVE,
	MCS_NEGATIVE,
	MCS_BYTES_SIZE,
	MCS_BYTES_CHUNK_SIZE,
	MCS_STRING_SIZE,
	MCS_STRING_CHUNK_SIZE,
	MCS_ARRAY_SIZE,
	MCS_MAP_SIZE,
	MCS_TAG,
	MCS_BYTES,
	MCS_BYTES_INDEF,
	MCS_BYTES_CHUNK,
	MCS_STRING,
	MCS_STRING_INDEF,
	MCS_STRING_CHUNK,
	MCS_SIMPLE,
	MCS_FLOAT,
	MCS_INVALID
} minicbor_state_t;

struct minicbor_reader_t {
	unsigned char Buffer[8];
	void (*PositiveFn)(void *UserData, uint64_t Number);
	void (*NegativeFn)(void *UserData, uint64_t Number);
	void (*BytesFn)(void *UserData, int Size);
	void (*BytesChunkFn)(void *UserData, void *Bytes, int Size, int Final);
	void (*StringFn)(void *UserData, int Size);
	void (*StringChunkFn)(void *UserData, void *Bytes, int Size, int Final);
	void (*ArrayFn)(void *UserData, int Size);
	void (*MapFn)(void *UserData, int Size);
	void (*TagFn)(void *UserData, uint64_t Tag);
	void (*FalseFn)(void *UserData);
	void (*TrueFn)(void *UserData);
	void (*NullFn)(void *UserData);
	void (*UndefinedFn)(void *UserData);
	void (*SimpleFn)(void *UserData, int Value);
	void (*FloatFn)(void *UserData, double Number);
	void (*BreakFn)(void *UserData);
	void (*ErrorFn)(void *UserData, int Position, const char *Message);
	void *UserData;
	int Position, Width, Required;
	minicbor_state_t State;
};

void minicbor_set_userdata(minicbor_reader_t *Reader, void *UserData) {
	Reader->UserData = UserData;
}

void minicbor_set_positive_fn(minicbor_reader_t *Reader, void (*PositiveFn)(void *UserData, uint64_t Number)) {
	Reader->PositiveFn = PositiveFn;
}

void minicbor_set_negative_fn(minicbor_reader_t *Reader, void (*NegativeFn)(void *UserData, uint64_t Number)) {
	Reader->NegativeFn = NegativeFn;
}

void minicbor_set_bytes_fn(minicbor_reader_t *Reader, void (*BytesFn)(void *UserData, int Size)) {
	Reader->BytesFn = BytesFn;
}

void minicbor_set_bytes_chunk_fn(minicbor_reader_t *Reader, void (*BytesChunkFn)(void *UserData, void *Bytes, int Size, int Final)) {
	Reader->BytesChunkFn = BytesChunkFn;
}

void minicbor_set_string_fn(minicbor_reader_t *Reader, void (*StringFn)(void *UserData, int Size)) {
	Reader->StringFn = StringFn;
}

void minicbor_set_string_chunk_fn(minicbor_reader_t *Reader, void (*StringChunkFn)(void *UserData, void *Bytes, int Size, int Final)) {
	Reader->StringChunkFn = StringChunkFn;
}

void minicbor_set_array_fn(minicbor_reader_t *Reader, void (*ArrayFn)(void *UserData, int Size)) {
	Reader->ArrayFn = ArrayFn;
}

void minicbor_set_map_fn(minicbor_reader_t *Reader, void (*MapFn)(void *UserData, int Size)) {
	Reader->MapFn = MapFn;
}

void minicbor_set_tag_fn(minicbor_reader_t *Reader, void (*TagFn)(void *UserData, uint64_t Tag)) {
	Reader->TagFn = TagFn;
}

void minicbor_set_false_fn(minicbor_reader_t *Reader, void (*FalseFn)(void *UserData)) {
	Reader->FalseFn = FalseFn;
}

void minicbor_set_true_fn(minicbor_reader_t *Reader, void (*TrueFn)(void *UserData)) {
	Reader->TrueFn = TrueFn;
}

void minicbor_set_null_fn(minicbor_reader_t *Reader, void (*NullFn)(void *UserData)) {
	Reader->NullFn = NullFn;
}

void minicbor_set_undefined_fn(minicbor_reader_t *Reader, void (*UndefinedFn)(void *UserData)) {
	Reader->UndefinedFn = UndefinedFn;
}

void minicbor_set_simple_fn(minicbor_reader_t *Reader, void (*SimpleFn)(void *UserData, int Value)) {
	Reader->SimpleFn = SimpleFn;
}

void minicbor_set_float_fn(minicbor_reader_t *Reader, void (*FloatFn)(void *UserData, double Number)) {
	Reader->FloatFn = FloatFn;
}

void minicbor_set_break_fn(minicbor_reader_t *Reader, void (*BreakFn)(void *UserData)) {
	Reader->BreakFn = BreakFn;
}

void minicbor_set_error_fn(minicbor_reader_t *Reader, void (*ErrorFn)(void *UserData, int Position, const char *Message)) {
	Reader->ErrorFn = ErrorFn;
}

void minicbor_read(minicbor_reader_t *Reader, unsigned char *Bytes, unsigned Available) {
	Reader->Position += Available;
	minicbor_state_t State = Reader->State;
	while (Available) switch (State) {
	case MCS_DEFAULT: {
		unsigned char Byte = *Bytes++;
		--Available;
		switch (Byte) {
		case 0x00 ... 0x17:
			Reader->PositiveFn(Reader->UserData, Byte - 0x00);
			break;
		case 0x18 ... 0x1B:
			Reader->Required = 1 << (Byte - 0x18);
			State = MCS_POSITIVE;
			break;
		case 0x20 ... 0x37:
			Reader->NegativeFn(Reader->UserData, Byte - 0x20);
			break;
		case 0x38 ... 0x3B:
			Reader->Required = 1 << (Byte - 0x38);
			State = MCS_NEGATIVE;
			break;
		case 0x40 ... 0x57:
			Reader->BytesFn(Reader->UserData, Byte - 0x40);
			Reader->Required = Byte - 0x40;
			State = MCS_BYTES;
			break;
		case 0x58 ... 0x5B:
			Reader->Required = 1 << (Byte - 0x58);
			State = MCS_BYTES_SIZE;
			break;
		case 0x5F:
			Reader->BytesFn(Reader->UserData, -1);
			State = MCS_BYTES_INDEF;
			break;
		case 0x60 ... 0x77:
			Reader->StringFn(Reader->UserData, Byte - 0x60);
			Reader->Required = Byte - 0x60;
			State = MCS_STRING;
			break;
		case 0x78 ... 0x7B:
			Reader->Required = 1 << (Byte - 0x78);
			State = MCS_STRING_SIZE;
			break;
		case 0x7F:
			Reader->BytesFn(Reader->UserData, -1);
			State = MCS_STRING_INDEF;
			break;
		case 0x80 ... 0x97:
			Reader->ArrayFn(Reader->UserData, Byte - 0x80);
			break;
		case 0x98 ... 0x9B:
			Reader->Required = 1 << (Byte - 0x98);
			State = MCS_ARRAY_SIZE;
			break;
		case 0x9F:
			Reader->ArrayFn(Reader->UserData, -1);
			break;
		case 0xA0 ... 0xB7:
			Reader->MapFn(Reader->UserData, Byte - 0xA0);
			break;
		case 0xB8 ... 0xBB:
			Reader->Required = 1 << (Byte - 0xB8);
			State = MCS_MAP_SIZE;
			break;
		case 0xBF:
			Reader->MapFn(Reader->UserData, -1);
			break;
		case 0xC0 ... 0xD7:
			Reader->TagFn(Reader->UserData, Byte - 0xC0);
			break;
		case 0xD8 ... 0xDB:
			Reader->Required = 1 << (Byte - 0xD8);
			State = MCS_TAG;
			break;
		case 0xF4:
			Reader->FalseFn(Reader->UserData);
			break;
		case 0xF5:
			Reader->TrueFn(Reader->UserData);
			break;
		case 0xF6:
			Reader->NullFn(Reader->UserData);
			break;
		case 0xF7:
			Reader->UndefinedFn(Reader->UserData);
			break;
		case 0xF8:
			State = MCS_SIMPLE;
			break;
		case 0xF9 ... 0xFB:
			Reader->Required = 1 << (Byte - 0xF8);
			State = MCS_FLOAT;
			break;
		case 0xFF:
			Reader->BreakFn(Reader->UserData);
			break;
		}
		break;
	}
	case MCS_POSITIVE: {
		int Required = Reader->Required;
		unsigned char *Next = Reader->Buffer + Reader->Width - Required;
		while (Available && Required) {
			*Next++ = *Bytes++;
			--Available;
			--Required;
		}
		if (!Required) {
			uint64_t Number = 0;
			switch (Reader->Width) {
			case 1: Number = *(uint8_t *)Reader->Buffer; break;
			case 2: Number = *(uint16_t *)Reader->Buffer; break;
			case 4: Number = *(uint32_t *)Reader->Buffer; break;
			case 8: Number = *(uint64_t *)Reader->Buffer; break;
			}
			Reader->PositiveFn(Reader->UserData, Number);
			State = MCS_DEFAULT;
		} else {
			Reader->Required = Required;
		}
		break;
	}
	case MCS_NEGATIVE: {
		int Required = Reader->Required;
		unsigned char *Next = Reader->Buffer + Reader->Width - Required;
		while (Available && Required) {
			*Next++ = *Bytes++;
			--Available;
			--Required;
		}
		if (!Required) {
			uint64_t Number = 0;
			switch (Reader->Width) {
			case 1: Number = *(uint8_t *)Reader->Buffer; break;
			case 2: Number = *(uint16_t *)Reader->Buffer; break;
			case 4: Number = *(uint32_t *)Reader->Buffer; break;
			case 8: Number = *(uint64_t *)Reader->Buffer; break;
			}
			Reader->NegativeFn(Reader->UserData, Number);
			State = MCS_DEFAULT;
		} else {
			Reader->Required = Required;
		}
		break;
	}
	case MCS_BYTES_SIZE: {
		int Required = Reader->Required;
		unsigned char *Next = Reader->Buffer + Reader->Width - Required;
		while (Available && Required) {
			*Next++ = *Bytes++;
			--Available;
			--Required;
		}
		if (!Required) {
			int Size = 0;
			switch (Reader->Width) {
			case 1: Size = *(uint8_t *)Reader->Buffer; break;
			case 2: Size = *(uint16_t *)Reader->Buffer; break;
			case 4: Size = *(uint32_t *)Reader->Buffer; break;
			case 8: Size = *(uint64_t *)Reader->Buffer; break;
			}
			Reader->BytesFn(Reader->UserData, Size);
			Reader->Required = Size;
			State = MCS_BYTES;
		} else {
			Reader->Required = Required;
		}
		break;
	}
	case MCS_BYTES_CHUNK_SIZE: {
		int Required = Reader->Required;
		unsigned char *Next = Reader->Buffer + Reader->Width - Required;
		while (Available && Required) {
			*Next++ = *Bytes++;
			--Available;
			--Required;
		}
		if (!Required) {
			int Size = 0;
			switch (Reader->Width) {
			case 1: Size = *(uint8_t *)Reader->Buffer; break;
			case 2: Size = *(uint16_t *)Reader->Buffer; break;
			case 4: Size = *(uint32_t *)Reader->Buffer; break;
			case 8: Size = *(uint64_t *)Reader->Buffer; break;
			}
			Reader->BytesFn(Reader->UserData, Size);
			Reader->Required = Size;
			State = MCS_BYTES_CHUNK;
		} else {
			Reader->Required = Required;
		}
		break;
	}
	case MCS_STRING_SIZE: {
		int Required = Reader->Required;
		unsigned char *Next = Reader->Buffer + Reader->Width - Required;
		while (Available && Required) {
			*Next++ = *Bytes++;
			--Available;
			--Required;
		}
		if (!Required) {
			int Size = 0;
			switch (Reader->Width) {
			case 1: Size = *(uint8_t *)Reader->Buffer; break;
			case 2: Size = *(uint16_t *)Reader->Buffer; break;
			case 4: Size = *(uint32_t *)Reader->Buffer; break;
			case 8: Size = *(uint64_t *)Reader->Buffer; break;
			}
			Reader->BytesFn(Reader->UserData, Size);
			Reader->Required = Size;
			State = MCS_STRING;
		} else {
			Reader->Required = Required;
		}
		break;
	}
	case MCS_STRING_CHUNK_SIZE: {
		int Required = Reader->Required;
		unsigned char *Next = Reader->Buffer + Reader->Width - Required;
		while (Available && Required) {
			*Next++ = *Bytes++;
			--Available;
			--Required;
		}
		if (!Required) {
			int Size = 0;
			switch (Reader->Width) {
			case 1: Size = *(uint8_t *)Reader->Buffer; break;
			case 2: Size = *(uint16_t *)Reader->Buffer; break;
			case 4: Size = *(uint32_t *)Reader->Buffer; break;
			case 8: Size = *(uint64_t *)Reader->Buffer; break;
			}
			Reader->BytesFn(Reader->UserData, Size);
			Reader->Required = Size;
			State = MCS_STRING_CHUNK;
		} else {
			Reader->Required = Required;
		}
		break;
	}
	case MCS_ARRAY_SIZE: {
		int Required = Reader->Required;
		unsigned char *Next = Reader->Buffer + Reader->Width - Required;
		while (Available && Required) {
			*Next++ = *Bytes++;
			--Available;
			--Required;
		}
		if (!Required) {
			int Size = 0;
			switch (Reader->Width) {
			case 1: Size = *(uint8_t *)Reader->Buffer; break;
			case 2: Size = *(uint16_t *)Reader->Buffer; break;
			case 4: Size = *(uint32_t *)Reader->Buffer; break;
			case 8: Size = *(uint64_t *)Reader->Buffer; break;
			}
			Reader->ArrayFn(Reader->UserData, Size);
			Reader->Required = Size;
			State = MCS_DEFAULT;
		} else {
			Reader->Required = Required;
		}
		break;
	}
	case MCS_MAP_SIZE: {
		int Required = Reader->Required;
		unsigned char *Next = Reader->Buffer + Reader->Width - Required;
		while (Available && Required) {
			*Next++ = *Bytes++;
			--Available;
			--Required;
		}
		if (!Required) {
			int Size = 0;
			switch (Reader->Width) {
			case 1: Size = *(uint8_t *)Reader->Buffer; break;
			case 2: Size = *(uint16_t *)Reader->Buffer; break;
			case 4: Size = *(uint32_t *)Reader->Buffer; break;
			case 8: Size = *(uint64_t *)Reader->Buffer; break;
			}
			Reader->MapFn(Reader->UserData, Size);
			Reader->Required = Size;
			State = MCS_DEFAULT;
		} else {
			Reader->Required = Required;
		}
		break;
	}
	case MCS_TAG: {
		int Required = Reader->Required;
		unsigned char *Next = Reader->Buffer + Reader->Width - Required;
		while (Available && Required) {
			*Next++ = *Bytes++;
			--Available;
			--Required;
		}
		if (!Required) {
			uint64_t Tag = 0;
			switch (Reader->Width) {
			case 1: Tag = *(uint8_t *)Reader->Buffer; break;
			case 2: Tag = *(uint16_t *)Reader->Buffer; break;
			case 4: Tag = *(uint32_t *)Reader->Buffer; break;
			case 8: Tag = *(uint64_t *)Reader->Buffer; break;
			}
			Reader->TagFn(Reader->UserData, Tag);
			State = MCS_DEFAULT;
		} else {
			Reader->Required = Required;
		}
		break;
	}
	case MCS_BYTES: {
		int Required = Reader->Required;
		if (Available < Required) {
			Reader->BytesChunkFn(Reader->UserData, Bytes, Available, 0);
			Reader->Required = Required - Available;
			Available = 0;
		} else {
			Reader->BytesChunkFn(Reader->UserData, Bytes, Required, 1);
			Reader->State = MCS_DEFAULT;
			Available -= Required;
		}
		break;
	}
	case MCS_BYTES_INDEF: {
		unsigned char Byte = Bytes[0];
		++Bytes;
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
			Reader->StringChunkFn(Reader->UserData, Bytes, 0, 1);
			State = MCS_DEFAULT;
			break;
		default:
			Reader->ErrorFn(Reader->UserData, Reader->Position - Available, "Invalid content in indefinite bytestring");
			State = MCS_INVALID;
			break;
		}
		break;
	}
	case MCS_BYTES_CHUNK: {
		int Required = Reader->Required;
		if (Available < Required) {
			Reader->BytesChunkFn(Reader->UserData, Bytes, Available, 0);
			Reader->Required = Required - Available;
			Available = 0;
		} else {
			Reader->BytesChunkFn(Reader->UserData, Bytes, Required, 0);
			Reader->State = MCS_BYTES_INDEF;
			Available -= Required;
		}
		break;
	}
	case MCS_STRING: {
		int Required = Reader->Required;
		if (Available < Required) {
			Reader->StringChunkFn(Reader->UserData, Bytes, Available, 0);
			Reader->Required = Required - Available;
			Available = 0;
		} else {
			Reader->StringChunkFn(Reader->UserData, Bytes, Required, 1);
			Reader->State = MCS_DEFAULT;
			Available -= Required;
		}
		break;
	}
	case MCS_STRING_INDEF: {
		unsigned char Byte = Bytes[0];
		++Bytes;
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
			Reader->StringChunkFn(Reader->UserData, Bytes, 0, 1);
			State = MCS_DEFAULT;
			break;
		default:
			Reader->ErrorFn(Reader->UserData, Reader->Position - Available, "Invalid content in indefinite string");
			State = MCS_INVALID;
			break;
		}
		break;
	}
	case MCS_STRING_CHUNK: {
		int Required = Reader->Required;
		if (Available < Required) {
			Reader->StringChunkFn(Reader->UserData, Bytes, Available, 0);
			Reader->Required = Required - Available;
			Available = 0;
		} else {
			Reader->StringChunkFn(Reader->UserData, Bytes, Required, 0);
			Reader->State = MCS_STRING_INDEF;
			Available -= Required;
		}
		break;
	}
	case MCS_SIMPLE: {
		int Value = *Bytes++;
		--Available;
		Reader->SimpleFn(Reader->UserData, Value);
		State = MCS_DEFAULT;
		break;
	}
	case MCS_FLOAT: {
		int Required = Reader->Required;
		unsigned char *Next = Reader->Buffer + Reader->Width - Required;
		while (Available && Required) {
			*Next++ = *Bytes++;
			--Available;
			--Required;
		}
		if (!Required) {
			double Number = 0;
			switch (Reader->Width) {
			case 2: {
				int Half = *(uint16_t *)Reader->Buffer;
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
			case 4: Number = *(float *)Reader->Buffer; break;
			case 8: Number = *(double *)Reader->Buffer; break;
			}
			Reader->FloatFn(Reader->UserData, Number);
			State = MCS_DEFAULT;
		} else {
			Reader->Required = Required;
		}
		break;
	}
	case MCS_INVALID: break;
	}
	Reader->State = State;
}

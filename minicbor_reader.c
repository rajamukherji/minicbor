#include "minicbor.h"
#include <math.h>

void minicbor_reader_init(minicbor_reader_t *Reader) {
	Reader->Position = 0;
	Reader->State = MCS_DEFAULT;
}

static inline uint64_t minicbor_read8(unsigned char *Bytes) {
	return Bytes[0];
}

static inline uint64_t minicbor_read16(unsigned char *Bytes) {
	uint64_t Number = Bytes[0];
	Number <<= 8;
	Number += Bytes[1];
	return Number;
}

static inline uint64_t minicbor_read32(unsigned char *Bytes) {
	uint64_t Number = Bytes[0];
	Number <<= 8;
	Number += Bytes[1];
	Number <<= 8;
	Number += Bytes[2];
	Number <<= 8;
	Number += Bytes[3];
	return Number;
}

static inline uint64_t minicbor_read64(unsigned char *Bytes) {
	uint64_t Number = Bytes[0];
	Number <<= 8;
	Number += Bytes[1];
	Number <<= 8;
	Number += Bytes[2];
	Number <<= 8;
	Number += Bytes[3];
	Number <<= 8;
	Number += Bytes[4];
	Number <<= 8;
	Number += Bytes[5];
	Number <<= 8;
	Number += Bytes[6];
	Number <<= 8;
	Number += Bytes[7];
	return Number;
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
		case 0xE0 ... 0xF7:
			Reader->SimpleFn(Reader->UserData, Byte - 0xE0);
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
			case 1: Number = minicbor_read8(Reader->Buffer); break;
			case 2: Number = minicbor_read16(Reader->Buffer); break;
			case 4: Number = minicbor_read32(Reader->Buffer); break;
			case 8: Number = minicbor_read64(Reader->Buffer); break;
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
			case 1: Number = minicbor_read8(Reader->Buffer); break;
			case 2: Number = minicbor_read16(Reader->Buffer); break;
			case 4: Number = minicbor_read32(Reader->Buffer); break;
			case 8: Number = minicbor_read64(Reader->Buffer); break;
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
			case 1: Size = minicbor_read8(Reader->Buffer); break;
			case 2: Size = minicbor_read16(Reader->Buffer); break;
			case 4: Size = minicbor_read32(Reader->Buffer); break;
			case 8: Size = minicbor_read64(Reader->Buffer); break;
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
			case 1: Size = minicbor_read8(Reader->Buffer); break;
			case 2: Size = minicbor_read16(Reader->Buffer); break;
			case 4: Size = minicbor_read32(Reader->Buffer); break;
			case 8: Size = minicbor_read64(Reader->Buffer); break;
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
			case 1: Size = minicbor_read8(Reader->Buffer); break;
			case 2: Size = minicbor_read16(Reader->Buffer); break;
			case 4: Size = minicbor_read32(Reader->Buffer); break;
			case 8: Size = minicbor_read64(Reader->Buffer); break;
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
			case 1: Size = minicbor_read8(Reader->Buffer); break;
			case 2: Size = minicbor_read16(Reader->Buffer); break;
			case 4: Size = minicbor_read32(Reader->Buffer); break;
			case 8: Size = minicbor_read64(Reader->Buffer); break;
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
			case 1: Size = minicbor_read8(Reader->Buffer); break;
			case 2: Size = minicbor_read16(Reader->Buffer); break;
			case 4: Size = minicbor_read32(Reader->Buffer); break;
			case 8: Size = minicbor_read64(Reader->Buffer); break;
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
			case 1: Size = minicbor_read8(Reader->Buffer); break;
			case 2: Size = minicbor_read16(Reader->Buffer); break;
			case 4: Size = minicbor_read32(Reader->Buffer); break;
			case 8: Size = minicbor_read64(Reader->Buffer); break;
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
			case 1: Tag = minicbor_read8(Reader->Buffer); break;
			case 2: Tag = minicbor_read16(Reader->Buffer); break;
			case 4: Tag = minicbor_read32(Reader->Buffer); break;
			case 8: Tag = minicbor_read64(Reader->Buffer); break;
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
				int Half = (Reader->Buffer[0] << 8) + Reader->Buffer[1];
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

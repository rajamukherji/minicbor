#include "minicbor.h"
#include <math.h>

#ifdef MINICBOR_WRITE_FN

extern void MINICBOR_WRITE_FN(minicbor_writedata_t UserData, const void *Bytes, unsigned Size);

static inline void minicbor_write(MINICBOR_WRITE_PARAMS, const void *Bytes, unsigned Size) {
	MINICBOR_WRITE_FN(UserData, Bytes, Size);
}

#define MINICBOR_WRITE_ARGS UserData

#else

static inline void minicbor_write(MINICBOR_WRITE_PARAMS, const void *Bytes, unsigned Size) {
	WriteFn(UserData, Bytes, Size);
}

#define MINICBOR_WRITE_ARGS UserData, WriteFn

#endif

static void minicbor_write_number(MINICBOR_WRITE_PARAMS, uint64_t Absolute, unsigned char Base) {
	unsigned char Bytes[9];
	if (Absolute < 24) {
		Bytes[0] = Base + Absolute;
		minicbor_write(MINICBOR_WRITE_ARGS, Bytes, 1);
	} else if (Absolute <= 0xFF) {
		Bytes[0] = Base + 0x18;
		Bytes[1] = Absolute & 0xFF;
		minicbor_write(MINICBOR_WRITE_ARGS, Bytes, 2);
	} else if (Absolute <= 0xFFFF) {
		Bytes[0] = Base + 0x19;
		Bytes[2] = Absolute & 0xFF;
		Absolute >>= 8;
		Bytes[1] = Absolute & 0xFF;
		minicbor_write(MINICBOR_WRITE_ARGS, Bytes, 3);
	} else if (Absolute <= 0xFFFFFFFF){
		Bytes[0] = Base + 0x1A;
		Bytes[4] = Absolute & 0xFF;
		Absolute >>= 8;
		Bytes[3] = Absolute & 0xFF;
		Absolute >>= 8;
		Bytes[2] = Absolute & 0xFF;
		Absolute >>= 8;
		Bytes[1] = Absolute & 0xFF;
		minicbor_write(MINICBOR_WRITE_ARGS, Bytes, 5);
	} else {
		Bytes[0] = Base + 0x1B;
		Bytes[8] = Absolute & 0xFF;
		Absolute >>= 8;
		Bytes[7] = Absolute & 0xFF;
		Absolute >>= 8;
		Bytes[6] = Absolute & 0xFF;
		Absolute >>= 8;
		Bytes[5] = Absolute & 0xFF;
		Absolute >>= 8;
		Bytes[4] = Absolute & 0xFF;
		Absolute >>= 8;
		Bytes[3] = Absolute & 0xFF;
		Absolute >>= 8;
		Bytes[2] = Absolute & 0xFF;
		Absolute >>= 8;
		Bytes[1] = Absolute & 0xFF;
		minicbor_write(MINICBOR_WRITE_ARGS, Bytes, 9);
	}
}

void minicbor_write_integer(MINICBOR_WRITE_PARAMS, int64_t Number) {
	if (Number < 0) {
		minicbor_write_number(MINICBOR_WRITE_ARGS, ~Number, 0x20);
	} else {
		minicbor_write_number(MINICBOR_WRITE_ARGS, Number, 0x0);
	}
}

void minicbor_write_positive(MINICBOR_WRITE_PARAMS, uint64_t Number) {
	minicbor_write_number(MINICBOR_WRITE_ARGS, Number, 0x0);
}

void minicbor_write_negative(MINICBOR_WRITE_PARAMS, uint64_t Number) {
	minicbor_write_number(MINICBOR_WRITE_ARGS, Number, 0x20);
}

void minicbor_write_bytes(MINICBOR_WRITE_PARAMS, unsigned Size) {
	minicbor_write_number(MINICBOR_WRITE_ARGS, Size, 0x40);
}

void minicbor_write_indef_bytes(MINICBOR_WRITE_PARAMS) {
	minicbor_write(MINICBOR_WRITE_ARGS, (unsigned char[]){0x5F}, 1);
}

void minicbor_write_string(MINICBOR_WRITE_PARAMS, unsigned Size) {
	minicbor_write_number(MINICBOR_WRITE_ARGS, Size, 0x60);
}

void minicbor_write_indef_string(MINICBOR_WRITE_PARAMS) {
	minicbor_write(MINICBOR_WRITE_ARGS, (unsigned char[]){0x7F}, 1);
}

void minicbor_write_array(MINICBOR_WRITE_PARAMS, unsigned Size) {
	minicbor_write_number(MINICBOR_WRITE_ARGS, Size, 0x80);
}

void minicbor_write_indef_array(MINICBOR_WRITE_PARAMS) {
	minicbor_write(MINICBOR_WRITE_ARGS, (unsigned char[]){0x9F}, 1);
}

void minicbor_write_map(MINICBOR_WRITE_PARAMS, unsigned Size) {
	minicbor_write_number(MINICBOR_WRITE_ARGS, Size, 0xA0);
}

void minicbor_write_indef_map(MINICBOR_WRITE_PARAMS) {
	minicbor_write(MINICBOR_WRITE_ARGS, (unsigned char[]){0xBF}, 1);
}

void minicbor_write_simple(MINICBOR_WRITE_PARAMS, unsigned char Simple) {
	if (Simple < 24) {
		minicbor_write(MINICBOR_WRITE_ARGS, (unsigned char[]){Simple + 0xE0}, 1);
	} else {
		minicbor_write(MINICBOR_WRITE_ARGS, (unsigned char[]){0xF8, Simple}, 2);
	}
}

void minicbor_write_float2(MINICBOR_WRITE_PARAMS, double Number) {
	char Bytes[3];
	Bytes[0] = 0xF9;
	// TODO: Implement this!
	minicbor_write(MINICBOR_WRITE_ARGS, Bytes, 3);
}

void minicbor_write_float4(MINICBOR_WRITE_PARAMS, double Number) {
	char Bytes[5];
	Bytes[0] = 0xFA;
	*(float *)(Bytes + 1) = Number;
	minicbor_write(MINICBOR_WRITE_ARGS, Bytes, 5);
}

void minicbor_write_float8(MINICBOR_WRITE_PARAMS, double Number) {
	char Bytes[9];
	Bytes[0] = 0xFB;
	*(double *)(Bytes + 1) = Number;
	minicbor_write(MINICBOR_WRITE_ARGS, Bytes, 9);
}

void minicbor_write_break(MINICBOR_WRITE_PARAMS) {
	minicbor_write(MINICBOR_WRITE_ARGS, (unsigned char[]){0xFF}, 1);
}

void minicbor_write_tag(MINICBOR_WRITE_PARAMS, uint64_t Tag) {
	minicbor_write_number(MINICBOR_WRITE_ARGS, Tag, 0xC0);
}


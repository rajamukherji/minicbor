#include "minicbor.h"
#include <math.h>

#ifdef MINICBOR_WRITE_FN

extern void MINICBOR_WRITE_FN(MINICBOR(writedata_t) UserData, const void *Bytes, unsigned Size);

static inline void MINICBOR(write)(MINICBOR_WRITE_PARAMS, const void *Bytes, unsigned Size) {
	MINICBOR_WRITE_FN(UserData, Bytes, Size);
}

#define MINICBOR_WRITE_ARGS UserData

#else

static inline void MINICBOR(write)(MINICBOR_WRITE_PARAMS, const void *Bytes, unsigned Size) {
	WriteFn(UserData, Bytes, Size);
}

#define MINICBOR_WRITE_ARGS UserData, WriteFn

#endif

static void MINICBOR(write_number)(MINICBOR_WRITE_PARAMS, uint64_t Absolute, unsigned char Base) {
	unsigned char Bytes[9];
	if (Absolute < 24) {
		Bytes[0] = Base + Absolute;
		MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 1);
	} else if (Absolute <= 0xFF) {
		Bytes[0] = Base + 0x18;
		Bytes[1] = Absolute & 0xFF;
		MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 2);
	} else if (Absolute <= 0xFFFF) {
		Bytes[0] = Base + 0x19;
		Bytes[2] = Absolute & 0xFF;
		Absolute >>= 8;
		Bytes[1] = Absolute & 0xFF;
		MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 3);
	} else if (Absolute <= 0xFFFFFFFF){
		Bytes[0] = Base + 0x1A;
		Bytes[4] = Absolute & 0xFF;
		Absolute >>= 8;
		Bytes[3] = Absolute & 0xFF;
		Absolute >>= 8;
		Bytes[2] = Absolute & 0xFF;
		Absolute >>= 8;
		Bytes[1] = Absolute & 0xFF;
		MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 5);
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
		MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 9);
	}
}

void MINICBOR(write_integer)(MINICBOR_WRITE_PARAMS, int64_t Number) {
	if (Number < 0) {
		MINICBOR(write_number)(MINICBOR_WRITE_ARGS, ~Number, 0x20);
	} else {
		MINICBOR(write_number)(MINICBOR_WRITE_ARGS, Number, 0x0);
	}
}

void MINICBOR(write_positive)(MINICBOR_WRITE_PARAMS, uint64_t Number) {
	MINICBOR(write_number)(MINICBOR_WRITE_ARGS, Number, 0x0);
}

void MINICBOR(write_negative)(MINICBOR_WRITE_PARAMS, uint64_t Number) {
	MINICBOR(write_number)(MINICBOR_WRITE_ARGS, Number, 0x20);
}

void MINICBOR(write_bytes)(MINICBOR_WRITE_PARAMS, unsigned Size) {
	MINICBOR(write_number)(MINICBOR_WRITE_ARGS, Size, 0x40);
}

void MINICBOR(write_indef_bytes)(MINICBOR_WRITE_PARAMS) {
	MINICBOR(write)(MINICBOR_WRITE_ARGS, (unsigned char[]){0x5F}, 1);
}

void MINICBOR(write_string)(MINICBOR_WRITE_PARAMS, unsigned Size) {
	MINICBOR(write_number)(MINICBOR_WRITE_ARGS, Size, 0x60);
}

void MINICBOR(write_indef_string)(MINICBOR_WRITE_PARAMS) {
	MINICBOR(write)(MINICBOR_WRITE_ARGS, (unsigned char[]){0x7F}, 1);
}

void MINICBOR(write_array)(MINICBOR_WRITE_PARAMS, unsigned Size) {
	MINICBOR(write_number)(MINICBOR_WRITE_ARGS, Size, 0x80);
}

void MINICBOR(write_indef_array)(MINICBOR_WRITE_PARAMS) {
	MINICBOR(write)(MINICBOR_WRITE_ARGS, (unsigned char[]){0x9F}, 1);
}

void MINICBOR(write_map)(MINICBOR_WRITE_PARAMS, unsigned Size) {
	MINICBOR(write_number)(MINICBOR_WRITE_ARGS, Size, 0xA0);
}

void MINICBOR(write_indef_map)(MINICBOR_WRITE_PARAMS) {
	MINICBOR(write)(MINICBOR_WRITE_ARGS, (unsigned char[]){0xBF}, 1);
}

void MINICBOR(write_simple)(MINICBOR_WRITE_PARAMS, unsigned char Simple) {
	if (Simple < 24) {
		MINICBOR(write)(MINICBOR_WRITE_ARGS, (unsigned char[]){Simple + 0xE0}, 1);
	} else {
		MINICBOR(write)(MINICBOR_WRITE_ARGS, (unsigned char[]){0xF8, Simple}, 2);
	}
}

void MINICBOR(write_float2)(MINICBOR_WRITE_PARAMS, double Number) {
	char Bytes[3];
	Bytes[0] = 0xF9;
	*(_Float16 *)(Bytes + 1) = Number;
	for (int I = 1; --I > 0;) {
		char T = Bytes[I];
		Bytes[I] = Bytes[3 - I];
		Bytes[3 - I] = T;
	}
	MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 3);
}

void MINICBOR(write_float4)(MINICBOR_WRITE_PARAMS, double Number) {
	char Bytes[5];
	Bytes[0] = 0xFA;
	*(float *)(Bytes + 1) = Number;
	for (int I = 3; --I > 0;) {
		char T = Bytes[I];
		Bytes[I] = Bytes[5 - I];
		Bytes[5 - I] = T;
	}
	MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 5);
}

void MINICBOR(write_float8)(MINICBOR_WRITE_PARAMS, double Number) {
	char Bytes[9];
	Bytes[0] = 0xFB;
	*(double *)(Bytes + 1) = Number;
	for (int I = 5; --I > 0;) {
		char T = Bytes[I];
		Bytes[I] = Bytes[9 - I];
		Bytes[9 - I] = T;
	}
	MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 9);
}

void MINICBOR(write_break)(MINICBOR_WRITE_PARAMS) {
	MINICBOR(write)(MINICBOR_WRITE_ARGS, (unsigned char[]){0xFF}, 1);
}

void MINICBOR(write_tag)(MINICBOR_WRITE_PARAMS, uint64_t Tag) {
	MINICBOR(write_number)(MINICBOR_WRITE_ARGS, Tag, 0xC0);
}


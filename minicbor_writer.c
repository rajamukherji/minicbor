#include "minicbor.h"
#include <math.h>

#ifdef MINICBOR_WRITE_FN

extern void MINICBOR_WRITE_FN(MINICBOR(writedata_t) UserData, const void *Bytes, size_t Size);

static inline void MINICBOR(write)(MINICBOR_WRITE_PARAMS, const void *Bytes, size_t Size) {
	MINICBOR_WRITE_FN(UserData, Bytes, Size);
}

#define MINICBOR_WRITE_ARGS UserData

#else

static inline void MINICBOR(write)(MINICBOR_WRITE_PARAMS, const void *Bytes, size_t Size) {
	WriteFn(UserData, Bytes, Size);
}

#define MINICBOR_WRITE_ARGS UserData, WriteFn

#endif

static void MINICBOR(write_number)(MINICBOR_WRITE_PARAMS, uint64_t Absolute, unsigned char Base) {
#ifdef MINICBOR_WRITE_BUFFER
	unsigned char *Bytes = MINICBOR_WRITE_BUFFER(UserData);
#else
	unsigned char Bytes[9];
#endif
	if (Absolute < 24) {
		Bytes[0] = Base + Absolute;
		MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 1);
	} else if (Absolute <= 0xFF) {
		Bytes[0] = Base + 0x18;
		Bytes[1] = Absolute;
		MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 2);
	} else if (Absolute <= 0xFFFF) {
		Bytes[0] = Base + 0x19;
		*(uint16_t *)(Bytes + 1) = __builtin_bswap16(Absolute);
		MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 3);
	} else if (Absolute <= 0xFFFFFFFF){
		Bytes[0] = Base + 0x1A;
		*(uint32_t *)(Bytes + 1) = __builtin_bswap32(Absolute);
		MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 5);
	} else {
		Bytes[0] = Base + 0x1B;
		*(uint64_t *)(Bytes + 1) = __builtin_bswap64(Absolute);
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
	static const unsigned char Bytes[] = {0x5F};
	MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 1);
}

void MINICBOR(write_string)(MINICBOR_WRITE_PARAMS, unsigned Size) {
	MINICBOR(write_number)(MINICBOR_WRITE_ARGS, Size, 0x60);
}

void MINICBOR(write_indef_string)(MINICBOR_WRITE_PARAMS) {
	static const unsigned char Bytes[] = {0x7F};
	MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 1);
}

void MINICBOR(write_array)(MINICBOR_WRITE_PARAMS, unsigned Size) {
	MINICBOR(write_number)(MINICBOR_WRITE_ARGS, Size, 0x80);
}

void MINICBOR(write_indef_array)(MINICBOR_WRITE_PARAMS) {
	static const unsigned char Bytes[] = {0x9F};
	MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 1);
}

void MINICBOR(write_map)(MINICBOR_WRITE_PARAMS, unsigned Size) {
	MINICBOR(write_number)(MINICBOR_WRITE_ARGS, Size, 0xA0);
}

void MINICBOR(write_indef_map)(MINICBOR_WRITE_PARAMS) {
	static const unsigned char Bytes[] = {0xBF};
	MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 1);
}

void MINICBOR(write_simple)(MINICBOR_WRITE_PARAMS, unsigned char Simple) {
#ifdef MINICBOR_WRITE_BUFFER
	unsigned char *Bytes = MINICBOR_WRITE_BUFFER(UserData);
#else
	unsigned char Bytes[2];
#endif
	if (Simple < 24) {
		Bytes[0] = Simple + 0xE0;
		MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 1);
	} else {
		Bytes[0] = 0xF8;
		Bytes[1] = Simple;
		MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 2);
	}
}

void MINICBOR(write_float2)(MINICBOR_WRITE_PARAMS, double Number) {
#ifdef MINICBOR_WRITE_BUFFER
	unsigned char *Bytes = MINICBOR_WRITE_BUFFER(UserData);
#else
	unsigned char Bytes[3];
#endif
	Bytes[0] = 0xF9;
	int Inf = isinf(Number);
	if (Inf < 0) {
		Bytes[1] = 0x00;
		Bytes[2] = 0xFC;
	} else if (Inf > 0) {
		Bytes[1] = 0x00;
		Bytes[2] = 0x7C;
	} else if (isnan(Number)) {
		Bytes[1] = 0x01;
		Bytes[2] = 0x7C;
	} else {
		int Sign = signbit(Number);
		int Exponent;
		int Mantissa = (int)(frexp(Number, &Exponent) * 2048) & 1023;
		Exponent += 15;
		if (Exponent < 0 || Exponent > 30) {
			Bytes[1] = 0x00;
			Bytes[2] = Sign ? 0xFC : 0x7C;
		} else {
			Bytes[1] = Mantissa & 255;
			Bytes[2] = (Mantissa >> 8) | (Exponent << 2) | (Sign << 7);
		}
	}
	MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 3);
}

void MINICBOR(write_float4)(MINICBOR_WRITE_PARAMS, double Number) {
#ifdef MINICBOR_WRITE_BUFFER
	unsigned char *Bytes = MINICBOR_WRITE_BUFFER(UserData);
#else
	unsigned char Bytes[5];
#endif
	Bytes[0] = 0xFA;
	*(float *)(Bytes + 1) = Number;
	for (int I = 3; --I > 0;) {
		unsigned char T = Bytes[I];
		Bytes[I] = Bytes[5 - I];
		Bytes[5 - I] = T;
	}
	MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 5);
}

void MINICBOR(write_float8)(MINICBOR_WRITE_PARAMS, double Number) {
#ifdef MINICBOR_WRITE_BUFFER
	unsigned char *Bytes = MINICBOR_WRITE_BUFFER(UserData);
#else
	unsigned char Bytes[9];
#endif
	Bytes[0] = 0xFB;
	*(double *)(Bytes + 1) = Number;
	for (int I = 5; --I > 0;) {
		unsigned char T = Bytes[I];
		Bytes[I] = Bytes[9 - I];
		Bytes[9 - I] = T;
	}
	MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 9);
}

void MINICBOR(write_break)(MINICBOR_WRITE_PARAMS) {
	static const unsigned char Bytes[] = {0x5F};
	MINICBOR(write)(MINICBOR_WRITE_ARGS, Bytes, 1);
}

void MINICBOR(write_tag)(MINICBOR_WRITE_PARAMS, uint64_t Tag) {
	MINICBOR(write_number)(MINICBOR_WRITE_ARGS, Tag, 0xC0);
}


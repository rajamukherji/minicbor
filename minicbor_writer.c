#include "minicbor.h"
#include <math.h>

static inline void minicbor_write(void *UserData, minicbor_write_fn WriteFn, const void *Bytes, unsigned Size) {
	WriteFn(UserData, Bytes, Size);
}

static void minicbor_write_number(void *UserData, minicbor_write_fn WriteFn, uint64_t Absolute, unsigned char Base) {
	unsigned char Bytes[9];
	if (Absolute < 24) {
		Bytes[0] = Base + Absolute;
		WriteFn(UserData, Bytes, 1);
	} else if (Absolute <= 0xFF) {
		Bytes[0] = Base + 0x18;
		*(uint8_t *)(Bytes + 1) = Absolute;
		WriteFn(UserData, Bytes, 2);
	} else if (Absolute <= 0xFFFF) {
		Bytes[0] = Base + 0x19;
		*(uint16_t *)(Bytes + 1) = Absolute;
		WriteFn(UserData, Bytes, 3);
	} else if (Absolute <= 0xFFFFFFFF){
		Bytes[0] = Base + 0x1A;
		*(uint32_t *)(Bytes + 1) = Absolute;
		WriteFn(UserData, Bytes, 5);
	} else {
		Bytes[0] = Base + 0x1B;
		*(uint64_t *)(Bytes + 1) = Absolute;
		WriteFn(UserData, Bytes, 9);
	}
}

void minicbor_write_integer(void *UserData, minicbor_write_fn WriteFn, int64_t Number) {
	if (Number < 0) {
		minicbor_write_number(UserData, WriteFn, ~Number, 0x20);
	} else {
		minicbor_write_number(UserData, WriteFn, Number, 0x0);
	}
}

void minicbor_write_positive(void *UserData, minicbor_write_fn WriteFn, uint64_t Number) {
	minicbor_write_number(UserData, WriteFn, Number, 0x0);
}

void minicbor_write_negative(void *UserData, minicbor_write_fn WriteFn, uint64_t Number) {
	minicbor_write_number(UserData, WriteFn, Number, 0x20);
}

void minicbor_write_bytes(void *UserData, minicbor_write_fn WriteFn, unsigned Size) {
	minicbor_write_number(UserData, WriteFn, Size, 0x40);
}

void minicbor_write_indef_bytes(void *UserData, minicbor_write_fn WriteFn) {
	WriteFn(UserData, (unsigned char[]){0x5F}, 1);
}

void minicbor_write_string(void *UserData, minicbor_write_fn WriteFn, unsigned Size) {
	minicbor_write_number(UserData, WriteFn, Size, 0x60);
}

void minicbor_write_indef_string(void *UserData, minicbor_write_fn WriteFn) {
	WriteFn(UserData, (unsigned char[]){0x7F}, 1);
}

void minicbor_write_array(void *UserData, minicbor_write_fn WriteFn, unsigned Size) {
	minicbor_write_number(UserData, WriteFn, Size, 0x80);
}

void minicbor_write_indef_array(void *UserData, minicbor_write_fn WriteFn) {
	WriteFn(UserData, (unsigned char[]){0x9F}, 1);
}

void minicbor_write_map(void *UserData, minicbor_write_fn WriteFn, unsigned Size) {
	minicbor_write_number(UserData, WriteFn, Size, 0xA0);
}

void minicbor_write_indef_map(void *UserData, minicbor_write_fn WriteFn) {
	WriteFn(UserData, (unsigned char[]){0xBF}, 1);
}

void minicbor_write_simple(void *UserData, minicbor_write_fn WriteFn, unsigned char Simple) {
	if (Simple < 24) {
		WriteFn(UserData, (unsigned char[]){Simple + 0xE0}, 1);
	} else {
		WriteFn(UserData, (unsigned char[]){0xF8, Simple}, 2);
	}
}

void minicbor_write_float2(void *UserData, minicbor_write_fn WriteFn, double Number) {
	char Bytes[3];
	Bytes[0] = 0xF9;
	// TODO: Implement this!
	WriteFn(UserData, Bytes, 3);
}

void minicbor_write_float4(void *UserData, minicbor_write_fn WriteFn, double Number) {
	char Bytes[5];
	Bytes[0] = 0xFA;
	*(float *)(Bytes + 1) = Number;
	WriteFn(UserData, Bytes, 5);
}

void minicbor_write_float8(void *UserData, minicbor_write_fn WriteFn, double Number) {
	char Bytes[9];
	Bytes[0] = 0xFB;
	*(double *)(Bytes + 1) = Number;
	WriteFn(UserData, Bytes, 9);
}

void minicbor_write_break(void *UserData, minicbor_write_fn WriteFn) {
	WriteFn(UserData, (unsigned char[]){0xFF}, 1);
}

void minicbor_write_tag(void *UserData, minicbor_write_fn WriteFn, uint64_t Tag) {
	minicbor_write_number(UserData, WriteFn, Tag, 0xC0);
}


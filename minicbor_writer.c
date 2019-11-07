#include "minicbor.h"
#include <stdlib.h>

#define new(T) (T*)malloc(sizeof(T))

struct minicbor_writer_t {
	void *UserData;
	minicbor_write_fn WriteFn;
	unsigned long Total;
};

minicbor_writer_t *minicbor_writer_new(void *UserData, minicbor_write_fn WriteFn) {
	minicbor_writer_t *Writer = new(minicbor_writer_t);
	Writer->UserData = UserData;
	Writer->WriteFn = WriteFn;
	Writer->Total = 0;
	return Writer;
}

static inline void minicbor_write(minicbor_writer_t *Writer, const void *Bytes, unsigned Size) {
	Writer->WriteFn(Writer->UserData, Bytes, Size);
	Writer->Total += Size;
}

static void minicbor_write_number(minicbor_writer_t *Writer, uint64_t Absolute, unsigned char Base) {
	unsigned char Bytes[9];
	if (Absolute < 24) {
		Bytes[0] = Base + Absolute;
		minicbor_write(Writer, Bytes, 1);
	} else if (Absolute <= 0xFF) {
		Bytes[0] = Base + 0x18;
		*(uint8_t *)(Bytes + 1) = Absolute;
		minicbor_write(Writer, Bytes, 2);
	} else if (Absolute <= 0xFFFF) {
		Bytes[0] = Base + 0x19;
		*(uint16_t *)(Bytes + 1) = Absolute;
		minicbor_write(Writer, Bytes, 3);
	} else if (Absolute <= 0xFFFFFFFF){
		Bytes[0] = Base + 0x1A;
		*(uint32_t *)(Bytes + 1) = Absolute;
		minicbor_write(Writer, Bytes, 5);
	} else {
		Bytes[0] = Base + 0x1B;
		*(uint64_t *)(Bytes + 1) = Absolute;
		minicbor_write(Writer, Bytes, 9);
	}
}

void minicbor_write_int(minicbor_writer_t *Writer, int64_t Number) {
	if (Number < 0) {
		minicbor_write_number(Writer, ~Number, 0x20);
	} else {
		minicbor_write_number(Writer, Number, 0x0);
	}
}

void minicbor_write_uint(minicbor_writer_t *Writer, uint64_t Number) {
	minicbor_write_number(Writer, Number, 0x0);
}

void minicbor_write_bytes(minicbor_writer_t *Writer, const void *String, unsigned Size) {
	minicbor_write_number(Writer, Size, 0x40);
	minicbor_write(Writer, String, Size);
}

void minicbor_write_bytes_open(minicbor_writer_t *Writer) {
	minicbor_write(Writer, (unsigned char[]){0x5F}, 1);
}

void minicbor_write_string(minicbor_writer_t *Writer, const char *String, unsigned Size) {
	minicbor_write_number(Writer, Size, 0x60);
	minicbor_write(Writer, String, Size);
}

void minicbor_write_indef_string(minicbor_writer_t *Writer) {
	minicbor_write(Writer, (unsigned char[]){0x7F}, 1);
}

void minicbor_write_array(minicbor_writer_t *Writer, unsigned Size) {
	minicbor_write_number(Writer, Size, 0x80);
}

void minicbor_write_indef_array(minicbor_writer_t *Writer) {
	minicbor_write(Writer, (unsigned char[]){0x9F}, 1);
}

void minicbor_write_map(minicbor_writer_t *Writer, unsigned Size) {
	minicbor_write_number(Writer, Size, 0xA0);
}

void minicbor_write_indef_map(minicbor_writer_t *Writer) {
	minicbor_write(Writer, (unsigned char[]){0xBF}, 1);
}

void minicbor_write_float(minicbor_writer_t *Writer, double Number) {
	char Bytes[9];
	// TODO: Add support for other float sizes
	Bytes[0] = 0xFB;
	*(double *)(Bytes + 1) = Number;
	minicbor_write(Writer, Bytes, 9);
}

void minicbor_write_false(minicbor_writer_t *Writer) {
	minicbor_write(Writer, (unsigned char[]){0xF4}, 1);
}

void minicbor_write_true(minicbor_writer_t *Writer) {
	minicbor_write(Writer, (unsigned char[]){0xF5}, 1);
}

void minicbor_write_null(minicbor_writer_t *Writer) {
	minicbor_write(Writer, (unsigned char[]){0xF6}, 1);
}

void minicbor_write_undef(minicbor_writer_t *Writer) {
	minicbor_write(Writer, (unsigned char[]){0xF7}, 1);
}

void minicbor_write_break(minicbor_writer_t *Writer) {
	minicbor_write(Writer, (unsigned char[]){0xFF}, 1);
}

void minicbor_write_tag(minicbor_writer_t *Writer, uint64_t Tag) {
	minicbor_write_number(Writer, Tag, 0xC0);
}


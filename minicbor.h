#ifndef MINICBOR_H
#define MINICBOR_H

#include <stdint.h>

#define CBOR_SIMPLE_FALSE 20
#define CBOR_SIMPLE_TRUE 21
#define CBOR_SIMPLE_NULL 22
#define CBOR_SIMPLE_UNDEF 23

typedef int (*minicbor_write_fn)(void *State, const void *UserData, unsigned Size);

void minicbor_write_integer(void *UserData, minicbor_write_fn WriteFn, int64_t Number);

void minicbor_write_positive(void *UserData, minicbor_write_fn WriteFn, uint64_t Number);
void minicbor_write_negative(void *UserData, minicbor_write_fn WriteFn, uint64_t Number);

void minicbor_write_bytes(void *UserData, minicbor_write_fn WriteFn, unsigned Size);
void minicbor_write_indef_bytes(void *UserData, minicbor_write_fn WriteFn);

void minicbor_write_string(void *UserData, minicbor_write_fn WriteFn, unsigned Size);
void minicbor_write_indef_string(void *UserData, minicbor_write_fn WriteFn);

void minicbor_write_array(void *UserData, minicbor_write_fn WriteFn, unsigned Size);
void minicbor_write_indef_array(void *UserData, minicbor_write_fn WriteFn);

void minicbor_write_map(void *UserData, minicbor_write_fn WriteFn, unsigned Size);
void minicbor_write_indef_map(void *UserData, minicbor_write_fn WriteFn);

void minicbor_write_float2(void *UserData, minicbor_write_fn WriteFn, double Number);
void minicbor_write_float4(void *UserData, minicbor_write_fn WriteFn, double Number);
void minicbor_write_float8(void *UserData, minicbor_write_fn WriteFn, double Number);

void minicbor_write_simple(void *UserData, minicbor_write_fn WriteFn, unsigned char Simple);

void minicbor_write_break(void *UserData, minicbor_write_fn WriteFn);

void minicbor_write_tag(void *UserData, minicbor_write_fn WriteFn, uint64_t Tag);


typedef struct minicbor_reader_t minicbor_reader_t;

typedef enum {
	MCS_DEFAULT,
	MCS_POSITIVE,
	MCS_NEGATIVE,
	MCS_BYTES_SIZE,
	MCS_BYTES,
	MCS_BYTES_INDEF,
	MCS_BYTES_CHUNK_SIZE,
	MCS_BYTES_CHUNK,
	MCS_STRING_SIZE,
	MCS_STRING,
	MCS_STRING_INDEF,
	MCS_STRING_CHUNK_SIZE,
	MCS_STRING_CHUNK,
	MCS_ARRAY_SIZE,
	MCS_MAP_SIZE,
	MCS_TAG,
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
	void (*SimpleFn)(void *UserData, int Value);
	void (*FloatFn)(void *UserData, double Number);
	void (*BreakFn)(void *UserData);
	void (*ErrorFn)(void *UserData, int Position, const char *Message);
	void *UserData;
	int Position, Width, Required;
	minicbor_state_t State;
};

void minicbor_reader_init(minicbor_reader_t *Reader);
void minicbor_read(minicbor_reader_t *Reader, unsigned char *Bytes, unsigned Available);

#endif

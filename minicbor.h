#ifndef MINICBOR_H
#define MINICBOR_H

#include <stdint.h>

typedef struct minicbor_writer_t minicbor_writer_t;
typedef int (*minicbor_write_fn)(void *State, const void *UserData, unsigned Size);

minicbor_writer_t *minicbor_writer_new(void *State, minicbor_write_fn WriteFn);

void minicbor_write_int(minicbor_writer_t *Writer, int64_t Number);
void minicbor_write_uint(minicbor_writer_t *Writer, uint64_t Number);

void minicbor_write_bytes(minicbor_writer_t *Writer, const void *Bytes, unsigned Size);
void minicbor_write_bytes_open(minicbor_writer_t *Writer);

void minicbor_write_string(minicbor_writer_t *Writer, const char *String, unsigned Size);
void minicbor_write_indef_string(minicbor_writer_t *Writer);

void minicbor_write_array(minicbor_writer_t *Writer, unsigned Size);
void minicbor_write_indef_array(minicbor_writer_t *Writer);

void minicbor_write_map(minicbor_writer_t *Writer, unsigned Size);
void minicbor_write_indef_map(minicbor_writer_t *Writer);

void minicbor_write_float(minicbor_writer_t *Writer, double Number);

void minicbor_write_false(minicbor_writer_t *Writer);
void minicbor_write_true(minicbor_writer_t *Writer);
void minicbor_write_null(minicbor_writer_t *Writer);
void minicbor_write_undef(minicbor_writer_t *Writer);

void minicbor_write_break(minicbor_writer_t *Writer);

void minicbor_write_tag(minicbor_writer_t *Writer, uint64_t Tag);


typedef struct minicbor_reader_t minicbor_reader_t;

minicbor_reader_t *minicbor_reader_new(void *UserData);
void minicbor_set_userdata(minicbor_reader_t *Reader, void *UserData);
void minicbor_set_positive_fn(minicbor_reader_t *Reader, void (*PositiveFn)(void *UserData, uint64_t Number));
void minicbor_set_negative_fn(minicbor_reader_t *Reader, void (*NegativeFn)(void *UserData, uint64_t Number));
void minicbor_set_bytes_fn(minicbor_reader_t *Reader, void (*BytesFn)(void *UserData, int Size));
void minicbor_set_bytes_chunk_fn(minicbor_reader_t *Reader, void (*BytesChunkFn)(void *UserData, void *Bytes, int Size, int Final));
void minicbor_set_string_fn(minicbor_reader_t *Reader, void (*StringFn)(void *UserData, int Size));
void minicbor_set_string_chunk_fn(minicbor_reader_t *Reader, void (*StringChunkFn)(void *UserData, void *Bytes, int Size, int Final));
void minicbor_set_array_fn(minicbor_reader_t *Reader, void (*ArrayFn)(void *UserData, int Size));
void minicbor_set_map_fn(minicbor_reader_t *Reader, void (*MapFn)(void *UserData, int Size));
void minicbor_set_tag_fn(minicbor_reader_t *Reader, void (*TagFn)(void *UserData, uint64_t Tag));
void minicbor_set_false_fn(minicbor_reader_t *Reader, void (*FalseFn)(void *UserData));
void minicbor_set_true_fn(minicbor_reader_t *Reader, void (*TrueFn)(void *UserData));
void minicbor_set_null_fn(minicbor_reader_t *Reader, void (*NullFn)(void *UserData));
void minicbor_set_undefined_fn(minicbor_reader_t *Reader, void (*UndefinedFn)(void *UserData));
void minicbor_set_simple_fn(minicbor_reader_t *Reader, void (*SimpleFn)(void *UserData, int Value));
void minicbor_set_float_fn(minicbor_reader_t *Reader, void (*FloatFn)(void *UserData, double Number));
void minicbor_set_break_fn(minicbor_reader_t *Reader, void (*BreakFn)(void *UserData));
void minicbor_set_error_fn(minicbor_reader_t *Reader, void (*ErrorFn)(void *UserData, int Position, const char *Message));
void minicbor_read(minicbor_reader_t *Reader, unsigned char *Bytes, unsigned Available);

#endif

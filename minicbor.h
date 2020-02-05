#ifndef MINICBOR_H
#define MINICBOR_H

#include <stdint.h>

#define MINICBOR_CONCAT2(X, Y) X ## Y
#define MINICBOR_CONCAT(X, Y) MINICBOR_CONCAT2(X, Y)

#ifndef MINICBOR_PREFIX
#define MINICBOR_PREFIX minicbor_
#endif

#define MINICBOR(SUFFIX) MINICBOR_CONCAT(MINICBOR_PREFIX, SUFFIX)

/**
 * Simple false value.
 */
#define CBOR_SIMPLE_FALSE 20

/**
 * Simple true value.
 */
#define CBOR_SIMPLE_TRUE 21

/**
 * Simple null value.
 */
#define CBOR_SIMPLE_NULL 22

/**
 * Simple undefined value.
 */
#define CBOR_SIMPLE_UNDEF 23

#ifdef MINICBOR_WRITEDATA_TYPE
typedef MINICBOR_WRITEDATA_TYPE MINICBOR(writedata_t);
#else
typedef void *MINICBOR(writedata_t);
#endif

#ifdef MINICBOR_WRITE_FN
#define MINICBOR_WRITE_PARAMS MINICBOR(writedata_t) UserData
#else
#define MINICBOR_WRITE_PARAMS MINICBOR(writedata_t) UserData, minicbor_write_fn WriteFn

/**
 * Minicbor write callback type.
 *
 * :param UserData: Pointer passed to :code:`minicbor_write_*()` functions.
 * :param Bytes: Bytes to write.
 * :param Size: Number of bytes.
 */
typedef int (*minicbor_write_fn)(MINICBOR(writedata_t) UserData, const unsigned char *Bytes, unsigned Size);

#endif

/**
 * Write a signed integer. Will automatically write a positive or negative integer with the smallest possible width.
 */
void MINICBOR(write_integer)(MINICBOR_WRITE_PARAMS, int64_t Number);

/**
 * Write a positive integer with the smallest width.
 */
void MINICBOR(write_positive)(MINICBOR_WRITE_PARAMS, uint64_t Number);

/**
 * Write a negative integer with the smallest width. Here `Number` is the exact value to write into the stream.
 * This means if :code:`X` is the desired negative value to write, then :code:`Number` should be :code:`1 - X` or :code:`~X` (the one's complement).
 * This is to allow the full range of negative numbers to be written.
 */
void MINICBOR(write_negative)(MINICBOR_WRITE_PARAMS, uint64_t Number);

/**
 * Write the leading bytes of a definite bytestring with :code:`Size` bytes.
 * The actual bytes should be written directly by the application.
 */
void MINICBOR(write_bytes)(MINICBOR_WRITE_PARAMS, unsigned Size);

/**
 * Write the leading bytes of an indefinite bytestring.
 * The chunks should be written using :c:func:`minicbor_write_bytes()` followed by the bytes themselves.
 * Finally, :c:func:`minicbor_write_break()` should be used to end the indefinite bytestring.
 */
void MINICBOR(write_indef_bytes)(MINICBOR_WRITE_PARAMS);

/**
 * Write the leading bytes of a definite string with :code:`Size` bytes.
 * The actual string should be written directly by the application.
 */
void MINICBOR(write_string)(MINICBOR_WRITE_PARAMS, unsigned Size);

/**
 * Write the leading bytes of an indefinite string.
 * The chunks should be written using :c:func:`minicbor_write_string()` followed by the strings themselves.
 * Finally, :c:func:`minicbor_write_break()` should be used to end the indefinite string.
 */
void MINICBOR(write_indef_string)(MINICBOR_WRITE_PARAMS);

/**
 * Write the leading bytes of a definite array with :code:`Size` elements.
 * The elements themselves should be written with the appropriate :code:`minicbor_write_*()` functions.
 */
void MINICBOR(write_array)(MINICBOR_WRITE_PARAMS, unsigned Size);

/**
 * Write the leading bytes of an indefinite array.
 * The elements themselves should be written with the appropriate :code:`minicbor_write_*()` functions.
 * Finally, :c:func:`minicbor_write_break()` should be used to ende the indefinite array.
 */
void MINICBOR(write_indef_array)(MINICBOR_WRITE_PARAMS);

/**
 * Write the leading bytes of a definite map with :code:`Size` key-value pairs.
 * The keys and values themselves should be written with the appropriate :code:`minicbor_write_*()` functions.
 */
void MINICBOR(write_map)(MINICBOR_WRITE_PARAMS, unsigned Size);

/**
 * Write the leading bytes of an indefinite map.
 * The keys and values themselves should be written with the appropriate :code:`minicbor_write_*()` functions.
 * Finally, :c:func:`minicbor_write_break()` should be used to ende the indefinite map.
 */
void MINICBOR(write_indef_map)(MINICBOR_WRITE_PARAMS);

/**
 * Write a floating point number in half precision.
 */
void MINICBOR(write_float2)(MINICBOR_WRITE_PARAMS, double Number);

/**
 * Write a floating point number in single precision.
 */
void MINICBOR(write_float4)(MINICBOR_WRITE_PARAMS, double Number);

/**
 * Write a floating point number in double precision.
 */
void MINICBOR(write_float8)(MINICBOR_WRITE_PARAMS, double Number);

/**
 * Write a simple value.
 */
void MINICBOR(write_simple)(MINICBOR_WRITE_PARAMS, unsigned char Simple);


/**
 * Write a break (to end an indefinite bytestring, string, array or map).
 */
void MINICBOR(write_break)(MINICBOR_WRITE_PARAMS);


/**
 * Write a tag sequence which will apply to the next value written.
 */
void MINICBOR(write_tag)(MINICBOR_WRITE_PARAMS, uint64_t Tag);

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
	MCS_INVALID,
	MCS_FINISHED
} minicbor_state_t;

#ifdef MINICBOR_READDATA_TYPE
typedef MINICBOR_READDATA_TYPE MINICBOR(readdata_t);
#else
typedef void *MINICBOR(readdata_t);
#endif

#ifdef MINICBOR_READ_FN_PREFIX

void MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, positive_fn)(MINICBOR(readdata_t) UserData, uint64_t Number);
void MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, negative_fn)(MINICBOR(readdata_t) UserData, uint64_t Number);
void MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, bytes_fn)(MINICBOR(readdata_t) UserData, int Size);
void MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, bytes_piece_fn)(MINICBOR(readdata_t) UserData, const void *Bytes, int Size, int Final);
void MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, string_fn)(MINICBOR(readdata_t) UserData, int Size);
void MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, string_piece_fn)(MINICBOR(readdata_t) UserData, const void *Bytes, int Size, int Final);
void MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, array_fn)(MINICBOR(readdata_t) UserData, int Size);
void MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, map_fn)(MINICBOR(readdata_t) UserData, int Size);
void MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, tag_fn)(MINICBOR(readdata_t) UserData, uint64_t Tag);
void MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, simple_fn)(MINICBOR(readdata_t) UserData, int Value);
void MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, float_fn)(MINICBOR(readdata_t) UserData, double Number);
void MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, break_fn)(MINICBOR(readdata_t) UserData);
void MINICBOR_CONCAT(MINICBOR_READ_FN_PREFIX, error_fn)(MINICBOR(readdata_t) UserData, int Position, const char *Message);

#else

typedef struct {
	/**
	 * Called when a positive integer is encountered.
	 */
	void (*PositiveFn)(MINICBOR(readdata_t) UserData, uint64_t Number);

	/**
	 * Called when a negative integer is encountered.
	 */
	void (*NegativeFn)(MINICBOR(readdata_t) UserData, uint64_t Number);

	/**
	 * Called when a bytestring is encountered.
	 * :code:`Size` is nonnegative for definite bytestrings and :code:`-1` for indefinite strings.
	 * For definite empty bytestrings, :code:`Size` is :code:`0` and :code:`BytesPieceFn()` is not called.
	 * Otherwise, :code:`BytesPieceFn()` will be called one or more times, with the last call having :code:`Final` set to :code:`1`.
	 */
	void (*BytesFn)(MINICBOR(readdata_t) UserData, int Size);

	/**
	 * Called for each piece of a bytestring.
	 * Note that pieces here do not correspond to CBOR chunks: there may be more pieces than chunks due to streaming.
	 */
	void (*BytesPieceFn)(MINICBOR(readdata_t) UserData, const void *Bytes, int Size, int Final);

	/**
	 * Called when a string is encountered.
	 * :code:`Size` is nonnegative for definite strings and :code:`-1` for indefinite strings.
	 * For definite empty strings, :code:`Size` is :code:`0` and :code:`StringPieceFn()` is not called.
	 * Otherwise, :code:`StringPieceFn()` will be called one or more times, with the last call having :code:`Final` set to :code:`1`.
	 */
	void (*StringFn)(MINICBOR(readdata_t) UserData, int Size);

	/**
	 * Called for each piece of a string.
	 * Note that pieces here do not correspond to CBOR chunks: there may be more pieces than chunks due to streaming.
	 */
	void (*StringPieceFn)(MINICBOR(readdata_t) UserData, const void *Bytes, int Size, int Final);

	/**
	 * Called when an array is encountered.
	 * :code:`Size` is nonnegative for definite array and :code:`-1` for indefinite arrays.
	 */
	void (*ArrayFn)(MINICBOR(readdata_t) UserData, int Size);

	/**
	 * Called when an map is encountered.
	 * :code:`Size` is nonnegative for definite map and :code:`-1` for indefinite maps.
	 */
	void (*MapFn)(MINICBOR(readdata_t) UserData, int Size);

	/**
	 * Called when a tag is encountered.
	 */
	void (*TagFn)(MINICBOR(readdata_t) UserData, uint64_t Tag);

	/**
	 * Called when a simple value is encounted.
	 */
	void (*SimpleFn)(MINICBOR(readdata_t) UserData, int Value);

	/**
	 * Called when a floating point number is encountered.
	 */
	void (*FloatFn)(MINICBOR(readdata_t) UserData, double Number);

	/**
	 *
	 * Called when a break is encountered.
	 * This is **not** called for breaks at the end of an indefinite bytestring or string, instead :code:`Final` is set to :code:`1` in the corresponding piece callback.
	 */
	void (*BreakFn)(MINICBOR(readdata_t) UserData);

	/**
	 * Called when an invalid CBOR sequence is detected.
	 * This puts the reader in an invalid state, any further calls will simply trigger another call :code:`ErrorFn()`;
	 */
	void (*ErrorFn)(MINICBOR(readdata_t) UserData, int Position, const char *Message);
} MINICBOR(reader_fns_t);

#endif

/**
 * A reader for a CBOR stream.
 * Must be initialized with :c:func:`minicbor_reader_init()` before use (and reuse).
 */
typedef struct minicbor_reader_t {
	/**
	 * Passed as the first argument to each callback.
	 */
	MINICBOR(readdata_t) UserData;

#ifndef MINICBOR_READ_FN_PREFIX
	MINICBOR(reader_fns_t) *Callbacks;
#endif

	unsigned char Buffer[8];
	int Position, Width, Required;
	minicbor_state_t State;
} minicbor_reader_t;

/**
 * Initializes :code:`Reader` for decoding a new CBOR stream.
 * Must be called before any call to :c:func:`minicbor_read()`.
 * A :c:type:`minicbor_reader_t` can be reused by calling this function again.
 */
void MINICBOR(reader_init)(minicbor_reader_t *Reader);

/**
 * Parse some CBOR bytes and call the appropriate callbacks.
 * Returns the 1 if :c:func:`minicbor_reader_finish()` was called within a callback, otherwise returns 0.
 */
int MINICBOR(read)(minicbor_reader_t *Reader, const unsigned char *Bytes, unsigned Size);

/**
 * Set :code:`Reader` state to :code:`MCS_FINISHED`.
 * Must be called from within a reader callback.
 */
void MINICBOR(reader_finish)(minicbor_reader_t *Reader);

/**
 * Returns the number of bytes remainining to be parsed by the reader.
 */
int MINICBOR(reader_remaining)(minicbor_reader_t *Reader);

#endif

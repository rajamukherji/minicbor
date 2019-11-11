#ifndef MINICBOR_H
#define MINICBOR_H

#include <stdint.h>

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

/**
 * Minicbor write callback type.
 *
 * :param UserData: Pointer passed to :code:`minicbor_write_*()` functions.
 * :param Bytes: Bytes to write.
 * :param Size: Number of bytes.
 */
typedef int (*minicbor_write_fn)(void *UserData, const void *Bytes, unsigned Size);

/**
 * Write a signed integer. Will automatically write a positive or negative integer with the smallest possible width.
 */
void minicbor_write_integer(void *UserData, minicbor_write_fn WriteFn, int64_t Number);

/**
 * Write a positive integer with the smallest width.
 */
void minicbor_write_positive(void *UserData, minicbor_write_fn WriteFn, uint64_t Number);

/**
 * Write a negative integer with the smallest width. Here `Number` is the exact value to write into the stream.
 * This means if :code:`X` is the desired negative value to write, then :code:`Number` should be :code:`1 - X` or :code:`~X` (the one's complement).
 * This is to allow the full range of negative numbers to be written.
 */
void minicbor_write_negative(void *UserData, minicbor_write_fn WriteFn, uint64_t Number);

/**
 * Write the leading bytes of a definite bytestring with :code:`Size` bytes.
 * The actual bytes should be written directly by the application.
 */
void minicbor_write_bytes(void *UserData, minicbor_write_fn WriteFn, unsigned Size);

/**
 * Write the leading bytes of an indefinite bytestring.
 * The chunks should be written using :c:func:`minicbor_write_bytes()` followed by the bytes themselves.
 * Finally, :c:func:`minicbor_write_break()` should be used to end the indefinite bytestring.
 */
void minicbor_write_indef_bytes(void *UserData, minicbor_write_fn WriteFn);

/**
 * Write the leading bytes of a definite string with :code:`Size` bytes.
 * The actual string should be written directly by the application.
 */
void minicbor_write_string(void *UserData, minicbor_write_fn WriteFn, unsigned Size);

/**
 * Write the leading bytes of an indefinite string.
 * The chunks should be written using :c:func:`minicbor_write_string()` followed by the strings themselves.
 * Finally, :c:func:`minicbor_write_break()` should be used to end the indefinite string.
 */
void minicbor_write_indef_string(void *UserData, minicbor_write_fn WriteFn);

/**
 * Write the leading bytes of a definite array with :code:`Size` elements.
 * The elements themselves should be written with the appropriate :code:`minicbor_write_*()` functions.
 */
void minicbor_write_array(void *UserData, minicbor_write_fn WriteFn, unsigned Size);

/**
 * Write the leading bytes of an indefinite array.
 * The elements themselves should be written with the appropriate :code:`minicbor_write_*()` functions.
 * Finally, :c:func:`minicbor_write_break()` should be used to ende the indefinite array.
 */
void minicbor_write_indef_array(void *UserData, minicbor_write_fn WriteFn);

/**
 * Write the leading bytes of a definite map with :code:`Size` key-value pairs.
 * The keys and values themselves should be written with the appropriate :code:`minicbor_write_*()` functions.
 */
void minicbor_write_map(void *UserData, minicbor_write_fn WriteFn, unsigned Size);

/**
 * Write the leading bytes of an indefinite map.
 * The keys and values themselves should be written with the appropriate :code:`minicbor_write_*()` functions.
 * Finally, :c:func:`minicbor_write_break()` should be used to ende the indefinite map.
 */
void minicbor_write_indef_map(void *UserData, minicbor_write_fn WriteFn);

/**
 * Write a floating point number in half precision.
 */
void minicbor_write_float2(void *UserData, minicbor_write_fn WriteFn, double Number);

/**
 * Write a floating point number in single precision.
 */
void minicbor_write_float4(void *UserData, minicbor_write_fn WriteFn, double Number);

/**
 * Write a floating point number in double precision.
 */
void minicbor_write_float8(void *UserData, minicbor_write_fn WriteFn, double Number);

/**
 * Write a simple value.
 */
void minicbor_write_simple(void *UserData, minicbor_write_fn WriteFn, unsigned char Simple);


/**
 * Write a break (to end an indefinite bytestring, string, array or map).
 */
void minicbor_write_break(void *UserData, minicbor_write_fn WriteFn);


/**
 * Write a tag sequence which will apply to the next value written.
 */
void minicbor_write_tag(void *UserData, minicbor_write_fn WriteFn, uint64_t Tag);

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

#ifdef MINICBOR_GLOBAL_FN_PREFIX

#define MINICBOR_CONCAT2(X, Y) X ## Y
#define MINICBOR_CONCAT(X, Y) MINICBOR_CONCAT2(X, Y)

void MINICBOR_CONCAT(MINICBOR_GLOBAL_FN_PREFIX, positive_fn)(void *UserData, uint64_t Number);
void MINICBOR_CONCAT(MINICBOR_GLOBAL_FN_PREFIX, negative_fn)(void *UserData, uint64_t Number);
void MINICBOR_CONCAT(MINICBOR_GLOBAL_FN_PREFIX, bytes_fn)(void *UserData, int Size);
void MINICBOR_CONCAT(MINICBOR_GLOBAL_FN_PREFIX, bytes_piece_fn)(void *UserData, void *Bytes, int Size, int Final);
void MINICBOR_CONCAT(MINICBOR_GLOBAL_FN_PREFIX, string_fn)(void *UserData, int Size);
void MINICBOR_CONCAT(MINICBOR_GLOBAL_FN_PREFIX, string_piece_fn)(void *UserData, void *Bytes, int Size, int Final);
void MINICBOR_CONCAT(MINICBOR_GLOBAL_FN_PREFIX, array_fn)(void *UserData, int Size);
void MINICBOR_CONCAT(MINICBOR_GLOBAL_FN_PREFIX, map_fn)(void *UserData, int Size);
void MINICBOR_CONCAT(MINICBOR_GLOBAL_FN_PREFIX, tag_fn)(void *UserData, uint64_t Tag);
void MINICBOR_CONCAT(MINICBOR_GLOBAL_FN_PREFIX, simple_fn)(void *UserData, int Value);
void MINICBOR_CONCAT(MINICBOR_GLOBAL_FN_PREFIX, float_fn)(void *UserData, double Number);
void MINICBOR_CONCAT(MINICBOR_GLOBAL_FN_PREFIX, break_fn)(void *UserData);
void MINICBOR_CONCAT(MINICBOR_GLOBAL_FN_PREFIX, error_fn)(void *UserData, int Position, const char *Message);

#endif

/**
 * A reader for a CBOR stream.
 * Must be initialized with :c:func:`minicbor_reader_init()` before use (and reuse).
 */
typedef struct minicbor_reader_t {
	/**
	 * Passed as the first argument to each callback.
	 */
	void *UserData;

#ifndef MINICBOR_GLOBAL_FN_PREFIX
	/**
	 * Called when a positive integer is encountered.
	 */
	void (*PositiveFn)(void *UserData, uint64_t Number);

	/**
	 * Called when a negative integer is encountered.
	 */
	void (*NegativeFn)(void *UserData, uint64_t Number);

	/**
	 * Called when a bytestring is encountered.
	 * :code:`Size` is nonnegative for definite bytestrings and :code:`-1` for indefinite strings.
	 * For definite empty bytestrings, :code:`Size` is :code:`0` and :code:`BytesPieceFn()` is not called.
	 * Otherwise, :code:`BytesPieceFn()` will be called one or more times, with the last call having :code:`Final` set to :code:`1`.
	 */
	void (*BytesFn)(void *UserData, int Size);

	/**
	 * Called for each piece of a bytestring.
	 * Note that pieces here do not correspond to CBOR chunks: there may be more pieces than chunks due to streaming.
	 */
	void (*BytesPieceFn)(void *UserData, void *Bytes, int Size, int Final);

	/**
	 * Called when a string is encountered.
	 * :code:`Size` is nonnegative for definite strings and :code:`-1` for indefinite strings.
	 * For definite empty strings, :code:`Size` is :code:`0` and :code:`StringPieceFn()` is not called.
	 * Otherwise, :code:`StringPieceFn()` will be called one or more times, with the last call having :code:`Final` set to :code:`1`.
	 */
	void (*StringFn)(void *UserData, int Size);

	/**
	 * Called for each piece of a string.
	 * Note that pieces here do not correspond to CBOR chunks: there may be more pieces than chunks due to streaming.
	 */
	void (*StringPieceFn)(void *UserData, void *Bytes, int Size, int Final);

	/**
	 * Called when an array is encountered.
	 * :code:`Size` is nonnegative for definite array and :code:`-1` for indefinite arrays.
	 */
	void (*ArrayFn)(void *UserData, int Size);

	/**
	 * Called when an map is encountered.
	 * :code:`Size` is nonnegative for definite map and :code:`-1` for indefinite maps.
	 */
	void (*MapFn)(void *UserData, int Size);

	/**
	 * Called when a tag is encountered.
	 */
	void (*TagFn)(void *UserData, uint64_t Tag);

	/**
	 * Called when a simple value is encounted.
	 */
	void (*SimpleFn)(void *UserData, int Value);

	/**
	 * Called when a floating point number is encountered.
	 */
	void (*FloatFn)(void *UserData, double Number);

	/**
	 *
	 * Called when a break is encountered.
	 * This is **not** called for breaks at the end of an indefinite bytestring or string, instead :code:`Final` is set to :code:`1` in the corresponding piece callback.
	 */
	void (*BreakFn)(void *UserData);

	/**
	 * Called when an invalid CBOR sequence is detected.
	 * This puts the reader in an invalid state, any further calls will simply trigger another call :code:`ErrorFn()`;
	 */
	void (*ErrorFn)(void *UserData, int Position, const char *Message);
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
void minicbor_reader_init(minicbor_reader_t *Reader);

/**
 * Parse some CBOR bytes and call the appropriate callbacks.
 */
void minicbor_read(minicbor_reader_t *Reader, unsigned char *Bytes, unsigned Size);

#endif

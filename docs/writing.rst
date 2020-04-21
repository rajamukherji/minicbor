Writing CBOR
============

Overview
--------

When an underlying stream type object is available, such as a file handle or an in-memory appendable buffer, simply pass a suitable :c:type:`minicbor_write_fn` to the :c:func:`minicbor_write_*` functions.

.. note::

   The :c:func:`minicbor_write_*` do not write the contents of any bytestring / string values. The contents of these values should be written directly by the user.

.. code-block:: c

   #include <minicbor.h>
   
   static void stream_write(stream_type *Stream, unsigned char *Bytes, int Size) {
      ...
   }
   
   void example_write() {
      stream_type *Stream = ...;
      minicbor_write_indef_array(Stream, stream, write);
      minicbor_write_string(Stream, stream_write, strlen("Hello world!"));
      stream_write(Stream, "Hello world!", strlen("Hello world!"));
      minicbor_write_integer(Stream, stream_write, 100);
      minicbor_write_float4(Stream, stream_write, 1.2);
      minicbor_write_break(Stream, stream_write);
   }

Presizing a CBOR output before writing
......................................

If a contiguous output buffer is required, then the required CBOR buffer size can be calculated by calling the :c:func:`minicbor_write_*` functions twice.

#. For the first pass, use a :c:type:`minicbor_write_fn` that takes a pointer to a :c:type:`size_t` and simply increments the value with the value of :c:data:`Size`. For example:

   .. code-block:: c

      static void calculate_size(size_t *Required, unsigned char *Bytes, int Size) {
         *Required += Size;
      }
   
   The user is responsible for incrementing :c:data:`Total` with the content sizes of any bytestrings or strings.

#. Then allocate a buffer (e.g. using :c:func:`malloc`) and use a :c:type:`minicbor_write_fn` that actual writes the data to the end of the buffer. For example:

   .. code-block:: c
   
      static void write_bytes(unsigned char **Tail, unsigned char *Bytes, int Size) {
         memcpy(*Tail, Bytes, Size);
         *Tail += Size;
      }

Defines
-------

.. c:macro:: CBOR_SIMPLE_FALSE

   Simple false value.

.. c:macro:: CBOR_SIMPLE_TRUE

   Simple true value.

.. c:macro:: CBOR_SIMPLE_NULL

   Simple null value.

.. c:macro:: CBOR_SIMPLE_UNDEF

   Simple undefined value.

Types
-----

.. c:type:: void (*minicbor_write_fn)(void *UserData, const void *Bytes, unsigned Size)

   Minicbor write callback type.

   :param UserData: Pointer passed to :code:`minicbor_write_*()` functions.
   :param Bytes: Bytes to write.
   :param Size: Number of bytes.

Functions
---------

.. c:function:: void minicbor_write_integer(void *UserData, minicbor_write_fn WriteFn, int64_t Number)

   Write a signed integer. Will automatically write a positive or negative integer with the smallest possible width.

.. c:function:: void minicbor_write_positive(void *UserData, minicbor_write_fn WriteFn, uint64_t Number)

   Write a positive integer with the smallest width.

.. c:function:: void minicbor_write_negative(void *UserData, minicbor_write_fn WriteFn, uint64_t Number)

   Write a negative integer with the smallest width. Here `Number` is the exact value to write into the stream.
   This means if :code:`X` is the desired negative value to write, then :code:`Number` should be :code:`1 - X` or :code:`~X` (the one's complement).
   This is to allow the full range of negative numbers to be written.

.. c:function:: void minicbor_write_bytes(void *UserData, minicbor_write_fn WriteFn, unsigned Size)

   Write the leading bytes of a definite bytestring with :code:`Size` bytes.
   The actual bytes should be written directly by the application.

.. c:function:: void minicbor_write_indef_bytes(void *UserData, minicbor_write_fn WriteFn)

   Write the leading bytes of an indefinite bytestring.
   The chunks should be written using :c:func:`minicbor_write_bytes()` followed by the bytes themselves.
   Finally, :c:func:`minicbor_write_break()` should be used to end the indefinite bytestring.

.. c:function:: void minicbor_write_string(void *UserData, minicbor_write_fn WriteFn, unsigned Size)

   Write the leading bytes of a definite string with :code:`Size` bytes.
   The actual string should be written directly by the application.

.. c:function:: void minicbor_write_indef_string(void *UserData, minicbor_write_fn WriteFn)

   Write the leading bytes of an indefinite string.
   The chunks should be written using :c:func:`minicbor_write_string()` followed by the strings themselves.
   Finally, :c:func:`minicbor_write_break()` should be used to end the indefinite string.


.. c:function:: void minicbor_write_array(void *UserData, minicbor_write_fn WriteFn, unsigned Size)

   Write the leading bytes of a definite array with :code:`Size` elements.
   The elements themselves should be written with the appropriate :code:`minicbor_write_*()` functions.

.. c:function:: void minicbor_write_indef_array(void *UserData, minicbor_write_fn WriteFn)

   Write the leading bytes of an indefinite array.
   The elements themselves should be written with the appropriate :code:`minicbor_write_*()` functions.
   Finally, :c:func:`minicbor_write_break()` should be used to ende the indefinite array.

.. c:function:: void minicbor_write_map(void *UserData, minicbor_write_fn WriteFn, unsigned Size) 

   Write the leading bytes of a definite map with :code:`Size` key-value pairs.
   The keys and values themselves should be written with the appropriate :code:`minicbor_write_*()` functions.

.. c:function:: void minicbor_write_indef_map(void *UserData, minicbor_write_fn WriteFn)

   Write the leading bytes of an indefinite map.
   The keys and values themselves should be written with the appropriate :code:`minicbor_write_*()` functions.
   Finally, :c:func:`minicbor_write_break()` should be used to ende the indefinite map.

.. c:function:: void minicbor_write_float2(void *UserData, minicbor_write_fn WriteFn, double Number)

   Write a floating point number in half precision.

.. c:function:: void minicbor_write_float4(void *UserData, minicbor_write_fn WriteFn, double Number)

   Write a floating point number in single precision.

.. c:function:: void minicbor_write_float8(void *UserData, minicbor_write_fn WriteFn, double Number)

   Write a floating point number in double precision.

.. c:function:: void minicbor_write_simple(void *UserData, minicbor_write_fn WriteFn, unsigned char Simple) 

   Write a simple value.

.. c:function:: void minicbor_write_break(void *UserData, minicbor_write_fn WriteFn) 

   Write a break (to end an indefinite bytestring, string, array or map).

.. c:function:: void minicbor_write_tag(void *UserData, minicbor_write_fn WriteFn, uint64t Tag)

   Write a tag sequence which will apply to the next value written.

Minicbor, a mini CBOR library
=============================

Overview
--------

Minicbor is a small C library for reading and writing CBOR encoded values.

Features
--------

* Suitable for streamed reading:

   Blocks of CBOR bytes can be parsed incrementally, even when a block splits a CBOR value.
   Callbacks allow the application to handle detected CBOR values / tokens as they are encountered.

* Suitable for streamed writing:

   The write functions are designed to work with an underlying application defined stream.

* No memory allocation making it easy to use Minicbor for developing language bindings.

Example
-------

Reading a CBOR stream
~~~~~~~~~~~~~~~~~~~~~

.. code-block:: c
   
   #include <minicbor.h>
   
   void example_read() {
      minicbor_reader_t Reader;
      
      // Initialize Reader
      minicbor_reader_init(&Reader);
      Reader.PositiveFn = ...;
      Reader.NegativeFn = ...;
      ...
      
      // Parse each block
      unsigned char Bytes[256];
      for (;;) {
         int Count = read(Stream, Bytes, 256);
         if (Count <= 0) break;
         minicbor_read(&Reader, Bytes, Size);
      }
   }

Writing a CBOR stream
~~~~~~~~~~~~~~~~~~~~~

When an underlying stream type object is available, such as a file handle or an in-memory appendable buffer, simply pass a suitable :c:type:`minicbor_write_fn` to the :c:func:`minicbor_write_*` functions.

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

#. For the first pass, use a :c:type:`minicbor_write_fn` that takes a pointer to a :c:type:`size_t` and simply increments the value with the value of :c:data:`Size`.
 For example:

   .. code-block:: c
   
      static void calculate_size(size_t *Required, unsigned char *Bytes, int Size) {
         *Required += Size;
      }
   
   Remember to increment :c:data:`Total` with the content sizes of any bytestrings or strings since there are no :c:func:`minicbor_write_*` for those.

#. Then allocate a buffer (e.g. using :c:func:`malloc`) and use a :c:type:`minicbor_write_fn` that actual writes the data to the end of the buffer. For example:

   .. code-block:: c
   
      static void write_bytes(unsigned char **Tail, unsigned char *Bytes, int Size) {
         memcpy(*Tail, Bytes, Size);
         *Tail += Size;
      }


.. toctree::
   :maxdepth: 2
   :caption: Contents:
   
   /reading
   /writing

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

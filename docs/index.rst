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
      minicbor_write_float4(Stream, stream_write,1.2);
      minicbor_write_break(Stream, stream_write);
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

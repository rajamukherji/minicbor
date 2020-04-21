Reading CBOR
============

Overview
--------

.. code-block:: c
   
   #include <minicbor.h>
   
   static minicbor_reader_fns Callbacks = {
      .PositiveFn = ...,
      .NegativeFn = ...,
      ...
      .ErrorFn = ...
   };
   
   void example_read() {
      minicbor_reader_t Reader;
      Reader.Callbacks = Callbacks;
      Reader.UserData = ...;
      
      // Initialize Reader
      minicbor_reader_init(&Reader);
   
      // Parse each block
      unsigned char Bytes[256];
      for (;;) {
         int Count = read(Stream, Bytes, 256);
         if (Count <= 0) break;
         minicbor_read(&Reader, Bytes, Size);
      }
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

.. c:type:: struct minicbor_reader_t

   A reader for a CBOR stream.
   Must be initialized with :c:func:`minicbor_reader_init()` before use (and reuse).

   .. c:member:: void *UserData
   
      Passed as the first argument to each callback.   
   
   .. c:member:: void (*PositiveFn)(void *UserData, uint64_t Number)

      Called when a positive integer is encountered.

   .. c:member:: void (*NegativeFn)(void *UserData, uint64_t Number) 

   Called when a negative integer is encountered.

   .. c:member:: void (*BytesFn)(void *UserData, int Size)
   
      Called when a bytestring is encountered.
      :code:`Size` is nonnegative for definite bytestrings and :code:`-1` for indefinite strings.
      For definite empty bytestrings, :c:data:`Size` is :code:`0` and :c:func:`BytesPieceFn()` is not called.
      Otherwise, :c:func:`BytesPieceFn()` will be called one or more times, with the last call having :c:data:`Final` set to :code:`1`.

   .. c:member:: void (*BytesPieceFn)(void *UserData, void *Bytes, int Size, int Final)
      
      Called for each piece of a bytestring.
      Note that pieces here do not correspond to CBOR chunks: there may be more pieces than chunks due to streaming.

   .. c:member:: void (*StringFn)(void *UserData, int Size)

      Called when a string is encountered.
      :code:`Size` is nonnegative for definite strings and :code:`-1` for indefinite strings.
      For definite empty strings, :c:data:`Size` is :code:`0` and :c:func:`StringPieceFn()` is not called.
      Otherwise, :c:func:`StringPieceFn()` will be called one or more times, with the last call having :c:data:`Final` set to :code:`1`.

   .. c:member:: void (*StringPieceFn)(void *UserData, void *Bytes, int Size, int Final)
   
      Called for each piece of a string.
      Note that pieces here do not correspond to CBOR chunks: there may be more pieces than chunks due to streaming.

   .. c:member:: void (*ArrayFn)(void *UserData, int Size)
   
      Called when an array is encountered.
      :c:data:`Size` is nonnegative for definite array and :code:`-1` for indefinite arrays.

   .. c:member:: void (*MapFn)(void *UserData, int Size)
   
      Called when an map is encountered.
      :c:data:`Size` is nonnegative for definite map and :code:`-1` for indefinite maps.

   .. c:member:: void (*TagFn)(void *UserData, uint64_t Tag)
   
      Called when a tag is encountered.

   .. c:member:: void (*SimpleFn)(void *UserData, int Value)

      Called when a simple value is encounted.

   .. c:member:: void (*FloatFn)(void *UserData, double Number) 

   Called when a floating point number is encountered.

   .. c:member:: void (*BreakFn)(void *UserData)
   
   Called when a break is encountered.
   This is **not** called for breaks at the end of an indefinite bytestring or string, instead :c:data:`Final` is set to :code:`1` in the corresponding piece callback.

   .. c:member:: void (*ErrorFn)(void *UserData, int Position, const char *Message)
   
      Called when an invalid CBOR sequence is detected.
      This puts the reader in an invalid state, any further calls will simply trigger another call :c:func:`ErrorFn()`;

Functions
---------

.. c:function:: void minicbor_reader_init(minicbor_reader_t *Reader)
   
   Initializes :c:data:`Reader` for decoding a new CBOR stream.
   Must be called before any call to :c:func:`minicbor_read()`.
   A :c:type:`minicbor_reader_t` can be reused by calling this function again.

.. c:function:: void minicbor_read(minicbor_reader_t *Reader, unsigned char *Bytes, unsigned Size)

   Parse some CBOR bytes and call the appropriate callbacks.

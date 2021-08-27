Minicbor, a mini CBOR library
=============================

Overview
--------

Minicbor is a small C library for reading and writing CBOR encoded values.

Features
--------

* Suitable for streamed reading: Blocks of CBOR bytes can be parsed incrementally, even when a block splits a CBOR value.
  Callbacks allow the application to handle detected CBOR values / tokens as they are encountered.
* Suitable for streamed writing: The write functions are designed to work with an underlying application defined stream.
* No memory allocation making it easy to use Minicbor for developing language bindings.

Usage
-----

The 3 files :file:`minicbor.h`, :file:`minicbor_reader.c` and :file:`minicbor_writer.c` can simply be dropped into an existing project. Alternatively library can be built if required using the supplied :file:`Makefile`.

By default the functions and types are all prefixed with :c:`minicbor_`. This can be changed by defining :c:`MINICBOR_PREFIX` when using the library.

License
-------

This library is released under the terms of the MIT license.

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

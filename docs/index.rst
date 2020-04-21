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

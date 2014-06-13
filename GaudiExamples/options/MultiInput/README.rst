MultiInput
----------

This example shows how to read data from two files in one event loop.

The option files ``PrepareBase.py`` and ``PrepareExt.py`` are used to generate
the input files for the actual example.

With the option file ``Read.py`` we read from the file created with
``PrepareBase.py`` extending the content of the TES with the entries coming from
the file produced with ``PrepareExt.py``.

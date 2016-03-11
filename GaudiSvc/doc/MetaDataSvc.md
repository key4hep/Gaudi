# MetaData service for self-descriptive n-tuples {#MetaDataSvc-readme}

The service adds information about n-tuple production into the resulting n-tuple.
These information include job options as well as application name and version.
They are named 'info' and saved inside the n-tuple.

## Usage
Metadata can be added to n-tuples adding Gaudi::MetaDataSvc to the list of extra
services in your options, for example with:

~~~{.py}
ApplicationMgr().ExtSvc  += [ "Gaudi::MetaDataSvc" ]
~~~

The content of the metadata info in a file can be dumped with the command line
tool `dumpMetaData` as in:

~~~
$ dumpMetaData TupleEx.root
~~~

An example on how to read the metadata from C++ is in the readMetaData.C example.

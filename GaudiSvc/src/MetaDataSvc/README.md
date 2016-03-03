<snippet>
  <content><![CDATA[


# ${1:MetaData service for self-descriptive n-tuples}

The service adds information about n-tuple production into the resulting n-tuple.
These information include application name and version and options used.
They are named 'info' and saved inside the n-tuple.

## Installation
The service is included in the option file in the following way:
ApplicationMgr().ExtSvc  += [ "MetaDataSvc" ]

## Usage
The info object can be opened and read with readMetaData.C script.
.x readMetaData.C
The script requires a new version of ROOT.

]]></content>
  <tabTrigger>readme</tabTrigger>
</snippet>

__version__ = "$Revision: 0.1 $"
__doc__ = """The python module holding python bindings to DataObjectDescriptor"""

FIELD_SEP = '|'
ITEM_SEP = '#'
ADDR_SEP = '&'

# s = "tracks|/evt/rec/tracks|0|0#hits|/evt/velo/hits|0|0#selTracks|/evt/mySel/tracks|0|1"

class DataObjectDescriptor(object):

    __slots__ = ('Tag', 'Path', 'AlternativePaths', 'Optional', 'AccessType')

    #define accessTypes
    READ = 0
    WRITE = 1
    UPDATE = 2

    def __init__(self, *args):
        for s in self.__slots__:
            setattr(self, s, None)

        #if we have one arg: initialize from string
        if len(args) is 1:
            self.fromString(args[0])
        else:
            #else the data is given
            self.fromArray(args)

    def fromString(self, s):
        if s == "":
            return

        fields = s.split(FIELD_SEP)
        self.fromArray(fields)

    def fromArray(self, a):
        if len(a) != 4:
            return

        self.Tag = a[0]

        if not ADDR_SEP in a[1]: #only one address provided
            self.Path = a[1]
            self.AlternativePaths = []
        else: #we have alternative addresses
            addr = a[1].split(ADDR_SEP)
            self.Path = addr[0]
            self.AlternativePaths = addr[1:]

        self.Optional = int(a[2]) == 1
        self.AccessType = int(a[3])

    def __str__(self):
        if not self.Tag:
            return ''
        return FIELD_SEP.join([self.Tag,
                               ADDR_SEP.join([self.Path] +
                                             self.AlternativePaths),
                               '1' if self.Optional else '0',
                               str(self.AccessType)])

    def __repr__(self):
        return "%s(%r)" % (self.__class__.__name__, str(self))

    def __eq__(self, other):
        return (self.__class__ == other.__class__ and
                all(getattr(self, s) == getattr(other, s)
                    for s in self.__slots__))

    def toStringProperty(self):
        return str(self)

class DataObjectDescriptorCollection(object):

    def __init__(self, dataItems):

        #dataItems is a string of format
        # "itemITEM_SEPitemITEM_SEP"
        # each item is a string of
        # "tagFIELD_SEPaddressFIELD_SEPoptionalFIELD_SEPaccessType"

        if dataItems == "":
            return

        items = dataItems.split(ITEM_SEP)

        for item in items:
            d = DataObjectDescriptor(item)
            object.__setattr__(self, d.Tag, d)

    def __getattr__(self, name):
        #is only called if attribute doesn't exist
        #don't allow creation of new DataItems
        raise AttributeError(name)

    def __setattr__(self, name, value):
        if name in self.__dict__:
            return object.__setattr__(self, name, value)
        else:
            #don't allow creation of new attributes
            raise AttributeError(name)

    def __str__(self):
        return ITEM_SEP.join(str(self.__dict__[key])
                             for key in sorted(self.__dict__))

    def __repr__(self):
        return "%s(%r)" % (self.__class__.__name__, str(self))

    def __eq__(self, other):
        # it's not very nice, but it's practical
        return self.__dict__ == other.__dict__

    def toStringProperty(self):
        return str(self)


# -*- coding: utf-8 -*-
"""
The Python module holding Python bindings to DataObjectDescriptor.
"""

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

        # possible values of args:
        # - (,)
        # - ('',)
        # - ('tag|path|0|0',)
        # - ('tag', 'path', False, READ)
        if args:
            # if we have one arg: initialize from string
            if len(args) == 1:
                if args[0]:
                    args = args[0].split(FIELD_SEP)
                else:
                    return # we are getting an empty string

            self.Tag, paths, opt, acc_type = args

            paths = paths.split(ADDR_SEP)
            self.Path = paths[0]
            self.AlternativePaths = paths[1:]

            self.Optional = bool(int(opt))

            self.AccessType = int(acc_type)

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

        if dataItems:
            for item in dataItems.split(ITEM_SEP):
                d = DataObjectDescriptor(item)
                object.__setattr__(self, d.Tag, d)

    def __setattr__(self, name, value):
        if name in self.__dict__:
            if isinstance(value, (basestring, tuple)):
                value = DataObjectDescriptor(value)
            if value.Tag != name:
                raise ValueError('cannot assign %r to %s (wrong tag)' % (value, name))
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
        try:
            return self.__dict__ == other.__dict__
        except AttributeError:
            return False

    def toStringProperty(self):
        return str(self)


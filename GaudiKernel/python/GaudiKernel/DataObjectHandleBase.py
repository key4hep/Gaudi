__version__ = "$Revision: 0.1 $"
__doc__ = """The python module holding python bindings to DataObjectHandle"""

# s = "/Path/to/Address"

class DataObjectHandleBase(object):

    __slots__ = ('Path', )

    #define accessTypes

    def __init__(self, path):
        object.__init__(self)
        self.Path = path

    def __eq__(self, other):
        """
        Need especially Configurable.isPropertySet when checked against default.
        """
        if isinstance(other, DataObjectHandleBase):
            return self.Path == other.Path
        if isinstance(other, basestring):
            return self.Path == other
        if other is None:
            return False
        raise ValueError('Unknown equality check: type=%r, repr=%r'%(type(other), other))

    def __ne__(self, other):
        """
        This is mandatory if __eq__ is defined.
        """
        return not self==other

    def __str__(self):
        return self.Path

    def __repr__(self):
        return "%s(\"%s\")" % (self.__class__.__name__, self.__str__())

    def toStringProperty(self):
        return self.__str__()

    def __add__(self,other):
        self.Path = ':'.join( i + other for i in self.Path.split(':'))
        return self

    def __radd__(self,other):
        self.Path = ':'.join( other + i for i in self.Path.split(':'))
        return self

    def __iadd__(self,other):
        self.Path = ':'.join( i + other for i in self.Path.split(':'))
        return self

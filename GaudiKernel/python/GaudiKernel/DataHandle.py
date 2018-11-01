__version__ = "$Revision: 0.1 $"
__doc__ = """The python module holding python bindings to DataHandle"""


class DataHandle(object):
    __slots__ = ('Path', '_whiteboard', '_access')

    # Access mode for DataHandles
    class AccessMode:
        class Read:
            pass

        class Write:
            pass

    # The default values allow PropertyProxy's _isCompatible check to pass when
    # a DataHandle is set from a path string.
    def __init__(self, path, whiteboard=None, access=None):
        object.__init__(self)
        self.Path = path
        self._whiteboard = whiteboard
        self._access = access

    def __eq__(self, other):
        if isinstance(other, DataHandle):
            return ((self.Path == other.Path) and
                    (self._whiteboard == other._whiteboard) and
                    (self._access == other._access))
        if isinstance(other, basestring):
            return self.Path == other
        if other is None:
            return False
        raise ValueError('Unknown equality check: type=%r, repr=%r' %
                         (type(other), other))

    def __ne__(self, other):
        """
        This is mandatory if __eq__ is defined.
        """
        return not self == other

    def __str__(self):
        return self.Path

    def __repr__(self):
        return "%s(\"path=%s, whiteboard=%s, access=%s\")" % (
            self.__class__.__name__,
            self.Path,
            self._whiteboard,
            self._access.__class__.__name__)

    def toStringProperty(self):
        return self.__str__()

    def __add__(self, other):
        path = ':'.join(i + other for i in self.Path.split(':'))
        return DataObjectHandleBase(path)

    def __radd__(self, other):
        path = ':'.join(other + i for i in self.Path.split(':'))
        return DataObjectHandleBase(path)

    def __iadd__(self, other):
        self.Path = ':'.join(i + other for i in self.Path.split(':'))
        return self

    def whiteboard(self):
        return self._whiteboard

    def access(self):
        return self._access

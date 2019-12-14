#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
__doc__ = """The python module holding python bindings to DataObjectHandle"""

# s = "/Path/to/Address"


class DataObjectHandleBase(object):

    __slots__ = ('Path', 'Mode', 'Type')

    # define accessTypes

    def __init__(self, path, mode='R', _type="unknown_t"):
        object.__init__(self)
        self.Path = path
        self.Mode = mode
        self.Type = _type

    def __getstate__(self):
        return {'Path': self.Path}

    def __setstate__(self, state):
        self.Path = state['Path']

    def __eq__(self, other):
        """
        Need especially Configurable.isPropertySet when checked against default.
        """
        if isinstance(other, DataObjectHandleBase):
            return self.Path == other.Path
        if isinstance(other, str):
            return self.Path == other
        if other is None:
            return False
        raise ValueError(
            'Unknown equality check: type=%r, repr=%r' % (type(other), other))

    def __ne__(self, other):
        """
        This is mandatory if __eq__ is defined.
        """
        return not self == other

    def __str__(self):
        return self.Path

    def __repr__(self):
        return "%s(\"%s\")" % (self.__class__.__name__, self.__str__())

    def toStringProperty(self):
        return self.__str__()

    def __add__(self, other):
        path = ':'.join(i + other for i in self.Path.split(':'))
        return DataObjectHandleBase(path, self.Mode, self.Type)

    def __radd__(self, other):
        path = ':'.join(other + i for i in self.Path.split(':'))
        return DataObjectHandleBase(path, self.Mode, self.Type)

    def __iadd__(self, other):
        self.Path = ':'.join(i + other for i in self.Path.split(':'))
        return self

    def mode(self):
        return self.Mode

    def type(self):
        return self.Type

    def __opt_value__(self):
        return repr(str(self))

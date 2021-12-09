#####################################################################################
# (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
__doc__ = """The python module holding python bindings to DataHandle"""


class DataHandle(object):

    __slots__ = ("Path", "Mode", "Type", "IsCondition")

    __hash__ = None  # Make class non-hashable for Python2 (default in Python3)

    def __init__(self, path, mode="R", _type="unknown_t", isCond=False):
        object.__init__(self)
        self.Path = path
        self.Mode = mode
        self.Type = _type
        self.IsCondition = isCond

    def __getstate__(self):
        return {s: getattr(self, s) for s in self.__slots__}

    def __setstate__(self, state):
        for s in state:
            setattr(self, s, state[s])

    def __eq__(self, other):
        """
        Need especially Configurable.isPropertySet when checked against default.
        """
        if isinstance(other, DataHandle):
            return self.__getstate__() == other.__getstate__()
        if isinstance(other, str):
            return self.Path == other
        if other is None:
            return False
        raise ValueError(
            "Unknown equality check: type=%r, repr=%r" % (type(other), other)
        )

    def __ne__(self, other):
        """
        This is mandatory if __eq__ is defined.
        """
        return not self == other

    def __str__(self):
        return self.Path

    def __repr__(self):
        args = "'%s','%s','%s'" % (self.Path, self.Mode, self.Type)
        if self.IsCondition:
            args += ",%s" % self.IsCondition
        return "%s(%s)" % (self.__class__.__name__, args)

    def toStringProperty(self):
        return self.__str__()

    def path(self):
        return self.Path

    def mode(self):
        return self.Mode

    def type(self):
        return self.Type

    def isCondition(self):
        return self.IsCondition

    def __opt_value__(self):
        return repr(str(self))

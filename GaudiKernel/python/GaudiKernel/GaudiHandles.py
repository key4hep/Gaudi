#####################################################################################
# (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################

import logging
from collections.abc import MutableSequence

# explicit list for wildcard imports
__all__ = [
    "GaudiHandle",
    "PublicToolHandle",
    "PrivateToolHandle",
    "ServiceHandle",
    "GaudiHandleArray",
    "ServiceHandleArray",
    "PublicToolHandleArray",
    "PrivateToolHandleArray",
]
__doc__ = """The python module holding python bindings to XyzHandles"""

_log = logging.getLogger("GaudiHandles")


class GaudiHandle(object):
    componentType = "Unspecified"  # must be overridden by derived class
    isPublic = True  # can be overridden by derived class

    __slots__ = ("typeAndName",)

    def __init__(self, typeAndName):
        object.__init__(self)
        if hasattr(typeAndName, "toStringProperty"):
            # this is a GaudiHandle or equivalent
            typeAndName = typeAndName.toStringProperty()
        if not isinstance(typeAndName, str):
            raise TypeError(
                "Argument to %s must be a string. Got a %s instead"
                % (self.__class__.__name__, type(typeAndName).__name__)
            )
        self.typeAndName = typeAndName

    def __repr__(self):
        return "%s(%r)" % (self.__class__.__name__, self.toStringProperty())

    def __str__(self):
        # FIXME: (Patch #1668) this creates problem with 2.5
        # return "%s:%s" % (self.__class__.__name__, self.toStringProperty())
        return self.toStringProperty()

    def __opt_value__(self):
        return self.toStringProperty()

    #
    # Several member functions which are the same as Configurables
    #
    def toStringProperty(self):
        return self.typeAndName

    def getType(self):
        """Get the 'type' part of the \"type/name\" string."""
        slash = self.typeAndName.find("/")
        if slash != -1:
            # includes name. Return part before /
            return self.typeAndName[0:slash]
        else:
            # only type is given, return full string
            return self.typeAndName

    def getName(self):
        """Get the 'name' part of the \"type/name\" string.
        If only a type is given, this will be returned as the name.
        If the \"type/name\" string is empty, it will return an emtpy string."""
        slash = self.typeAndName.find("/")
        if slash != -1:
            # explicit name. Return part after the /
            return self.typeAndName[slash + 1 :]
        else:
            # only type is given. return type as default name
            return self.typeAndName

    def getGaudiHandle(self):
        return self

    def getFullName(self):
        return self.toStringProperty()

    def __eq__(self, other):
        return type(self) is type(other) and self.typeAndName == other.typeAndName


class PublicToolHandle(GaudiHandle):
    __slots__ = ()
    componentType = "AlgTool"
    isPublic = True

    def __init__(self, toolTypeAndName=""):
        GaudiHandle.__init__(self, toolTypeAndName)


class PrivateToolHandle(GaudiHandle):
    __slots__ = ()
    componentType = "AlgTool"
    isPublic = False

    def __init__(self, toolTypeAndName=""):
        GaudiHandle.__init__(self, toolTypeAndName)


class ServiceHandle(GaudiHandle):
    __slots__ = ()
    componentType = "Service"
    isPublic = True

    def __init__(self, serviceName=""):
        GaudiHandle.__init__(self, serviceName)


#
# The HandleArrays
#


class GaudiHandleArray(MutableSequence):
    """A unique list of GaudiHandles. Only handles of one type are allowed,
    as specified by self.__class__.handleType"""

    __slots__ = ("_items",)
    handleType = None  # must be set by derived class to the handle type

    def __init__(self, typesAndNames=None):
        # Use of dictionary as storage for faster duplicate finding
        self._items = {}  # name : handle

        # check the type
        if typesAndNames is None:
            typesAndNames = []
        elif not isinstance(typesAndNames, list):
            raise TypeError(
                "Argument to %s must be a list. Got a %s instead"
                % (self.__class__.__name__, type(typesAndNames).__name__)
            )
        # add entries to list
        self.extend(typesAndNames)

    def __len__(self):
        return len(self._items)

    def __eq__(self, other):
        try:
            return self._items == other._items
        except AttributeError:
            # other is e.g. plain list
            return list(self._items.values()) == other

    def __repr__(self):
        """Return class name with list of type/name strings as argument"""
        return (
            self.__class__.__name__
            + "(["
            + ",".join(repr(h.toStringProperty()) for h in self._items.values())
            + "])"
        )

    def __str__(self):
        """Print in a form which can be parsed"""
        return self.toStringProperty()

    def _indexToKey(self, index):
        """Convert list index to dictionary key"""
        return list(self._items.keys())[index]

    def __copy__(self):
        inst = self.__class__.__new__(self.__class__)
        inst._items = self._items.copy()
        return inst

    def __setitem__(self, index, value):
        try:
            self._items[self._indexToKey(index)] = value
        except TypeError as e:
            if isinstance(index, slice):
                raise TypeError(
                    "Setting elements of %s via slice is not supported"
                    % self.__class__.__name__
                )
            else:
                raise e

    def __getitem__(self, index):
        if isinstance(index, str):
            # search by instance name
            try:
                return self._items[index]
            except KeyError:
                raise IndexError(
                    "%s does not have a %s with instance name %s"
                    % (self.__class__.__name__, self.handleType.componentType, index)
                )
        else:
            if isinstance(index, slice):
                return [self._items[k] for k in self._indexToKey(index)]
            else:
                return self._items[self._indexToKey(index)]

    def __iter__(self):
        yield from self._items.values()

    def __delitem__(self, index):
        if isinstance(index, slice):
            for k in self._indexToKey(index):
                self._items.pop(k)
        else:
            self._items.pop(self._indexToKey(index))

    def __contains__(self, value):
        if isinstance(value, str):
            return value in self._items
        else:
            return value.getName() in self._items

    def __add__(self, other):
        return self.__class__(list(self) + list(other))

    def insert(self, index, value):
        """Only allow inserting compatible types. It accepts a string, a handle or a configurable."""
        if isinstance(value, str):
            # convert string to handle
            value = self.__class__.handleType(value)
        elif isinstance(value, self.__class__.handleType):
            pass  # leave handle as-is
        elif isinstance(value, GaudiHandle):
            # do not allow different type of handles
            raise TypeError(
                "Can not add a %s to a %s"
                % (value.__class__.__name__, self.__class__.__name__)
            )
        elif value.getGaudiType() != self.__class__.handleType.componentType:
            # assume it is a configurable: allow only correct types
            raise TypeError(
                "Can not append %s (%s) to a %s"
                % (
                    value.__class__.__name__,
                    value.getGaudiType(),
                    self.__class__.__name__,
                )
            )
        elif hasattr(value, "isPublic"):
            # check public vs private if applicable for this configurable
            pop = value.isPublic() and "Public" or "Private"
            if value.isPublic() != self.__class__.handleType.isPublic:
                raise TypeError(
                    "Can not append %s (%s %s) to a %s"
                    % (
                        value.__class__.__name__,
                        pop,
                        value.getGaudiType(),
                        self.__class__.__name__,
                    )
                )

        # check that an instance name appears only once in the list
        if value.getName() not in self._items:
            if index == len(self._items):  # append
                self._items[value.getName()] = value
            else:
                # for `insert` we need to rebuild the dictionary in the correct order
                values = list(self._items.items())
                values.insert(index, (value.getName(), value))
                self._items = dict(values)
        else:
            oldValue = self._items[value.getName()]
            _log.warning(
                "%r with instance name %r already in %s. Not adding %r.",
                oldValue,
                oldValue.getName(),
                self.__class__.__name__,
                value,
            )

    def isPublic(self):
        return self.__class__.handleType.isPublic

    #
    # Member functions which are the same as Configurables
    #
    def toStringProperty(self):
        return (
            "["
            + ",".join(repr(v.toStringProperty()) for v in self._items.values())
            + "]"
        )


class ServiceHandleArray(GaudiHandleArray):
    __slots__ = ()
    handleType = ServiceHandle

    def __init__(self, serviceTypesAndNames=None):
        GaudiHandleArray.__init__(self, serviceTypesAndNames)


class PublicToolHandleArray(GaudiHandleArray):
    __slots__ = ()
    handleType = PublicToolHandle

    def __init__(self, toolTypesAndNames=None):
        GaudiHandleArray.__init__(self, toolTypesAndNames)


class PrivateToolHandleArray(GaudiHandleArray):
    __slots__ = ()
    handleType = PrivateToolHandle

    def __init__(self, toolTypesAndNames=None):
        GaudiHandleArray.__init__(self, toolTypesAndNames)

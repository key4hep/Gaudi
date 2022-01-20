#####################################################################################
# (c) Copyright 1998-2022 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# Classes and functions for handling Configurables
from __future__ import absolute_import

import sys

_GLOBAL_INSTANCES = False


def useGlobalInstances(enable):
    """
    Enable or disable the global instances database.

    By default global instances are enabled.
    """
    global _GLOBAL_INSTANCES
    if enable == _GLOBAL_INSTANCES:
        return
    if not enable:
        assert (
            not Configurable.instances
        ), "Configurable instances DB not empty, cannot be disabled"
    _GLOBAL_INSTANCES = enable


class Property(object):
    """
    Descriptor class to implement validation of Configurable properties.
    """

    def __init__(self, cpp_type, default, doc="undocumented", semantics=None):
        from .semantics import getSemanticsFor

        self.semantics = getSemanticsFor(semantics or cpp_type)
        self.default = default
        self.__doc__ = doc

    @property
    def cpp_type(self):
        return self.semantics.cpp_type

    @property
    def name(self):
        return self.semantics.name

    def __get__(self, instance, owner):
        if self.name not in instance._properties and hasattr(self.semantics, "default"):
            instance._properties[self.name] = self.semantics.default(self.default)
        return self.semantics.load(instance._properties.get(self.name, self.default))

    def __set__(self, instance, value):
        instance._properties[self.name] = self.semantics.store(value)

    def __delete__(self, instance):
        del instance._properties[self.name]

    def __set_name__(self, owner, name):
        self.semantics.name = name

    def __is_set__(self, instance, owner):
        try:
            value = instance._properties[self.name]
            return self.semantics.is_set(value)
        except KeyError:
            return False

    def __opt_value__(self, instance, owner):
        return self.semantics.opt_value(
            instance._properties.get(self.name, self.default)
        )

    def __merge__(self, instance, owner, value):
        """
        Return "merge" (according to the semantic) of the value
        in this property and the incoming value.
        """
        if not self.__is_set__(instance, owner):
            return value
        return self.semantics.merge(self.__get__(instance, owner), value)


class ConfigurableMeta(type):
    """
    Metaclass for Configurables.
    """

    def __new__(cls, name, bases, namespace, **kwds):
        props = {
            key: namespace[key]
            for key in namespace
            if isinstance(namespace[key], Property)
        }
        if props:
            doc = namespace.get("__doc__", "").rstrip()
            doc += "\n\nProperties\n----------\n"
            doc += "\n".join(
                [
                    "- {name}: {p.cpp_type} ({p.default!r})\n  {p.__doc__}\n".format(
                        name=n, p=props[n]
                    )
                    for n in props
                ]
            )
            namespace["__doc__"] = doc
            if sys.version_info < (3, 6):  # pragma no cover
                for n in props:
                    namespace[n].__set_name__(None, n)
            namespace["_descriptors"] = props
        slots = set(namespace.get("__slots__", []))
        slots.update(["_properties", "_name"])
        namespace["__slots__"] = tuple(slots)
        result = type.__new__(cls, name, bases, namespace)
        return result


def opt_repr(value):
    """
    String representation of the value, such that it can be consumed be the
    Gaudi option parsers.
    """
    if hasattr(value, "__opt_repr__"):
        return value.__opt_repr__()
    elif isinstance(value, str):
        return '"{}"'.format(value.replace('"', '\\"'))
    return repr(value)


if sys.version_info >= (3,):  # pragma no cover
    exec("class ConfigMetaHelper(metaclass=ConfigurableMeta):\n pass")
else:  # pragma no cover

    class ConfigMetaHelper(object):
        __metaclass__ = ConfigurableMeta


class Configurable(ConfigMetaHelper):
    """
    Base class for all configurable instances.
    """

    instances = {}

    def __init__(self, name=None, **kwargs):
        self._name = None
        self._properties = {}
        if "parent" in kwargs:
            parent = kwargs.pop("parent")
            if isinstance(parent, basestring if sys.version_info[0] == 2 else str):
                parent = self.instances[parent]
            if not name:
                raise TypeError("name is needed when a parent is specified")
            name = "{}.{}".format(parent.name, name)
        if name:
            self.name = name
        elif not _GLOBAL_INSTANCES:
            self.name = self.__cpp_type__
        for key, value in kwargs.items():
            setattr(self, key, value)

    @classmethod
    def getInstance(cls, name):
        return cls.instances.get(name) or cls(name)

    @property
    def name(self):
        if not self._name:
            raise AttributeError(
                "{!r} instance was not named yet".format(type(self).__name__)
            )
        return self._name

    @name.setter
    def name(self, value):
        if value == self._name:
            return  # it's already the name of the instance, nothing to do
        if (
            not isinstance(value, basestring if sys.version_info[0] == 2 else str)
            or not value
        ):
            raise TypeError(
                "expected string, got {} instead".format(type(value).__name__)
            )
        if _GLOBAL_INSTANCES:
            if value in self.instances:
                raise ValueError("name {!r} already used".format(value))
            if self._name in self.instances:
                del self.instances[self._name]
            self._name = value
            self.instances[value] = self
        else:
            self._name = value

    @name.deleter
    def name(self):
        if _GLOBAL_INSTANCES:
            # check if it was set
            del self.instances[self.name]
            self._name = None
        else:
            raise TypeError("name attribute cannot be deleted")

    def __repr__(self):
        args = []
        try:
            args.append(repr(self.name))
        except AttributeError:
            pass  # no name
        args.extend("{}={!r}".format(*item) for item in self._properties.items())
        return "{}({})".format(type(self).__name__, ", ".join(args))

    def __getstate__(self):
        state = {"properties": self._properties}
        try:
            state["name"] = self.name
        except AttributeError:
            pass  # no name
        return state

    def __setstate__(self, state):
        self._name = None
        self.name = state.get("name")
        self._properties = state["properties"]

    def __opt_value__(self):
        if self.__cpp_type__ == self.name:
            return self.__cpp_type__
        return "{}/{}".format(self.__cpp_type__, self.name)

    def __opt_properties__(self, explicit_defaults=False):
        name = self.name
        out = {}
        for p in self._descriptors.values():
            if explicit_defaults or p.__is_set__(self, type(self)):
                out[".".join([name, p.name])] = opt_repr(
                    p.__opt_value__(self, type(self))
                )
        return out

    def is_property_set(self, propname):
        return self._descriptors[propname].__is_set__(self, type(self))

    @classmethod
    def getGaudiType(cls):
        return cls.__component_type__

    @classmethod
    def getType(cls):
        return cls.__cpp_type__

    def getName(self):
        return self.name

    def getFullJobOptName(self):
        return "{}/{}".format(self.__cpp_type__, self.name)

    def toStringProperty(self):
        return "{}/{}".format(self.__cpp_type__, self.name)

    @classmethod
    def getDefaultProperties(cls):
        return {k: v.default for k, v in cls._descriptors.items()}

    @classmethod
    def getDefaultProperty(cls, name):
        return cls._descriptors[name].default

    def merge(self, other):
        """
        Merge the properties of the other instance into the current one.

        The two instances have to be of the same type, have the same name
        (or both unnamed) and the settings must be mergable (according to
        their semantics).
        """
        if type(self) is not type(other):
            raise TypeError(
                "cannot merge instance of {} into an instance of {}".format(
                    type(other).__name__, type(self).__name__
                )
            )
        if hasattr(self, "name") != hasattr(other, "name"):
            raise ValueError("cannot merge a named configurable with an unnamed one")
        if hasattr(self, "name") and (self.name != other.name):
            raise ValueError(
                "cannot merge configurables with different names ({} and {})".format(
                    self.name, other.name
                )
            )

        for name in other._descriptors:
            if not other.is_property_set(name):
                continue
            try:
                setattr(
                    self,
                    name,
                    self._descriptors[name].__merge__(
                        self, type(self), getattr(other, name)
                    ),
                )
            except ValueError as err:
                raise ValueError(
                    "conflicting settings for property {} of {}: {}".format(
                        name,
                        self.name if hasattr(self, "name") else type(self).__name__,
                        str(err),
                    )
                )

        return self


def makeConfigurableClass(name, **namespace):
    """
    Create a Configurable specialization.
    """
    properties = namespace.pop("properties", {})
    namespace.update({pname: Property(*pargs) for pname, pargs in properties.items()})

    return type(name, (Configurable,), namespace)


def all_options(explicit_defaults=False):
    """
    Return a dictionary with all explicitly set options, or with also the
    defaults if explicit_defaults is set to True.
    """
    opts = {}
    for c in Configurable.instances.values():
        opts.update(c.__opt_properties__(explicit_defaults))
    return opts

"""
This module contains the main classes for the Python counterparts of Gaudi C++
components used for the configuration: the "Configurable"s.

Configurables are "named singletons", in the sense that they are instantiated
with a name (by default the class name) and only one instance can exist with the
given name.
Two calls to the constructor of a class with the same name will return the same
instance (even if called from different modules in the same process).
There cannot be instances of different types with the same name. Trying to
instantiate an object of a type with the name of an already existing object of
another type with raise an exception. A possible work-around is

  name = old_instance.name
  old_instance.name = name + "_old"
  new_instance = NewType(name, **old_instance.propertyDict())


The shape of objects and classes are unmutable. It is not possible to add data
members or methods to the classes or instances outside the definition.

Configurable classes must provide the special class member "__properties__" which
must be an iterable containing instances of the Property class (in the submodule
Properties).

@author: Marco Clemencic <marco.clemencic@cern.ch>
"""

import Properties

class MetaConfigurable(type):
    """
    Meta-class for Configurables.

    Instrument the Configurable class with the properties defined in
    __properties__.
    The optional __cpp_type__ can be used to specify the corresponding C++ type
    (if not present, the Python class name is used).
    """
    def __new__(cls, name, bases, dict):
        """
        Add the property data members to the generated class and fill the __slots__
        data member accordingly.
        """
        # get the slots from the base class
        slots = set(dict.get("__slots__", []))
        # if there are no properties defined, add anyway the __properties__
        # member
        if "__properties__" not in dict:
            dict["__properties__"] = []
        # add the requested properties
        for p in dict["__properties__"]:
            dict[p.name] = property(p.get, p.set, p.delete, p.doc)
            slots.add(p.name)
        # update the slots
        dict["__slots__"] = tuple(slots)
        # Add the __cpp_type__ property if not defined
        if "__cpp_type__" not in dict:
            dict["__cpp_type__"] = name
        # set the default instance name if not defined
        if "__defaultInstanceName__" not in dict:
            dict["__defaultInstanceName__"] = dict["__cpp_type__"]
        # generate the class
        return type.__new__(cls, name, bases, dict)

    def __setattr__(self, name, value):
        """
        A __setattr__ declaration here forbids modifications of the class object.
        """
        raise AttributeError("'%s' class has no attribute '%s'" % (self.__name__, name))

class Configurable(object):
    """
    Base class for configurables.
    """
    __metaclass__ = MetaConfigurable
    __slots__ = ("_name", "_propertyData")
    # All instantiated configurables
    _instances = {}

    def _getName(self):
        """
        Getter for property 'name'.
        """
        return self._name
    def _setName(self, name):
        """
        Setter for property 'name'.
        """
        if name not in self._instances:
            # rename allowed
            # remove old entry
            del self._instances[self._name]
            # add self with the new name
            self._name = name
            self._instances[self._name] = self
        else:
            raise ValueError("cannot rename '%s' to '%s', name already used"
                             % (self._name, name))
    def _delName(self):
        """
        Delete handler for property 'name'.
        """
        raise AttributeError("cannot remove attribute 'name'")

    name = property(_getName, _setName, _delName, "Name of the instance")

    def __new__(cls, name = None, **kwargs):
        # Instance default name
        if name is None:
            name = cls.__defaultInstanceName__
        # check if the name is already used
        if name not in cls._instances:
            # a new instance is needed
            instance = super(Configurable, cls).__new__(cls)
            cls._instances[name] = instance
            # Initialize data members of the instance
            #  - name
            instance._name = name
            #  - storage for the properties that are set
            instance._propertyData = {}
        else:
            # instance already present
            instance = cls._instances[name]
            # check if the type matches
            if instance.__class__ is not cls:
                raise TypeError("Instance called '%s' already exists of class '%s' ('%s' requested)"
                                % (name, instance.__class__.__name__, cls.__name__))
        # set the properties from keyword arguments
        for attr, value in kwargs.items():
            setattr(instance, attr, value)
        return instance

    def __repr__(self):
        """
        Return representation string for the configurable instance.

        The string can be evaluated to obtain an equivalent instance.
        """
        retval = self.__class__.__name__ + "("
        if self.name != self.__class__.__defaultInstanceName__:
            pars = [repr(self.name)]
        else:
            pars = []
        retval += ",".join(pars + [ "%s=%r" % i for i in self._propertyData.items()])
        retval += ")"
        return retval

    def __str__(self):
        """
        String representation as <C++ class>/<instance name>.
        """
        return "/".join((self.__cpp_type__, self.name))

    def isSet(self, prop):
        """
        Tell if the property has been explicitly set.
        """
        return prop in self._propertyData

    @classmethod
    def getDefault(cls, prop):
        """
        Return the default value of a property.
        """
        for p in cls.__properties__:
            if p.name == prop:
                return p.default
        # If not in the class, try the bases
        for base in [ b for b in cls.__bases__ if hasattr(b, "getDefault") ]:
            try:
                return base.getDefault(prop)
            except AttributeError:
                # hide errors from the base classes
                pass
        raise AttributeError("'%s' object has no property '%s'"
                             % (cls.__name__, prop))

    @classmethod
    def propertyNames(cls):
        """
        Return a list with the names of all the properties.
        """
        names = [ p.name for p in cls.__properties__ ]
        for base in [ b for b in cls.__bases__ if hasattr(b, "propertyNames") ]:
            names += base.propertyNames()
        names = list(set(names)) # uniquify
        names.sort()
        return names

    def propertyDict(self, defaults = False):
        """
        Return a dictionary with the value of the properties.

        @param defaults: if True, the properties that are not set are included
        in the returned dictionary with the default value, if False, only the
        properties that are explicitly set are returned.
        """
        d = dict(self._propertyData)
        if defaults:
            for prop in [ p for p in self.propertyNames() if p not in d ]:
                d[prop] = self.getDefault(prop)
        return d

    def __getnewargs__(self):
        """
        Needed for pickling with protocol 2.
        """
        # return the instance name in a tuple, so that it is passed to the
        # __new__ method.
        return (self.name,)
    def __getstate__(self):
        """
        Required for pickling with protocol 2.
        (otherwise the pickling uses the __slots__, which implies that the defaults
        get set)
        """
        return self.propertyDict()
    def __setstate__(self, state):
        """
        Required for pickling with protocol 2 (to match __getstate__).
        """
        for prop, value in state.items():
            setattr(self, prop, value)

class Service(Configurable):
    """
    Base class for standard component type "Service".
    """
    pass

class Algorithm(Configurable):
    """
    Base class for standard component type "Algorithm".
    """
    pass

class AlgTool(Configurable):
    """
    Base class for standard component type "AlgTool".
    """
    pass

class Auditor(Configurable):
    """
    Base class for standard component type "Auditor".
    """
    pass

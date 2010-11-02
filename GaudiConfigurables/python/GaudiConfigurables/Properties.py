'''
Created on 28/mag/2010

@author: Marco Clemencic
'''
import os, sys, logging

# FIXME: This is needed for properties of type handle.
from GaudiKernel.GaudiHandles import *

# Number of times to indent output
# A list is used to force access by reference
__report_indent = [0]
def report(fn):
    """Decorator to print information about a function
    call for use while debugging.
    Prints function name, arguments, and call number
    when the function is called. Prints this information
    again along with the return value when the function
    returns.
    """
    def wrap(*params,**kwargs):
        call = wrap.callcount = wrap.callcount + 1
        indent = ' ' * __report_indent[0]
        fc = "%s(%s)" % (fn.__name__, ', '.join(
            [a.__repr__() for a in params] +
            ["%s = %s" % (a, repr(b)) for a,b in kwargs.items()]
        ))
        print "%s%s called [#%s]" % (indent, fc, call)
        __report_indent[0] += 1
        ret = fn(*params,**kwargs)
        __report_indent[0] -= 1
        print "%s%s returned %s [#%s]" % (indent, fc, repr(ret), call)
        return ret
    wrap.callcount = 0
    return wrap

def _defaultValidator(_):
    """Default no-op validation function."""
    return True

class ValidatorsDB(object):
    """Class to dynamically load the validator modules.
    """
    def __init__(self):
        self._log = logging.getLogger("ValidatorsDB")
        self._modIter = self._modules()

    def _modules(self):
        """Iterator over the list of validator modules.
        """
        # collect the list of validator modules
        suff = "_validators."
        if sys.platform.startswith("win"):
            suff += "pyd"
        else:
            suff += "so"
        # actual loop
        for d in sys.path:
            if not d: d = '.' # empty name means '.'
            elif not os.path.isdir(d): # skip non-directories (elif because '' is a directory)
                continue
            self._log.debug("Scanning directory %s", d)
            for m in ( os.path.splitext(f)[0]
                       for f in os.listdir(d)
                       if f.endswith(suff) ):
                yield m

    def __getattr__(self, name):
        """Import the validator modules until the validator for the requested
        type is found.
        """
        self._log.debug("Requested validator for unknown type %s", name)
        # loop over the "other" modules
        for m in self._modIter:
            # import
            self._log.debug("Importing %s", m)
            m = __import__(m)
            # add the validators to the instance scope
            for v in dir(m):
                self._log.debug("Registering type %s", v)
                if not v.startswith("_"):
                    self.__dict__[v] = getattr(m, v)
            # check if now we have what was requested
            if name in self.__dict__:
                self._log.debug("Found")
                return self.__dict__[name]
        self._log.debug("Not found, using default")
        self.__dict__[name] = _defaultValidator
        return _defaultValidator

_validatorsDB = ValidatorsDB()

class Property(object):
    '''
    Generic property class to handle
    '''
    def __init__(self, name,
                 cppType, default,
                 validator = _defaultValidator, doc = None):
        '''
        Constructor
        '''
        self.name = name
        self.cppType = cppType
        if type(validator) is str:
            validator = getattr(_validatorsDB, validator)
        self._validator = validator

        if self._validator(repr(default)):
            self.default = default
        else:
            raise TypeError("Bad default value %r: %s" %
                            (default, self._validator.__doc__))

        # The documentation of a property is:
        #  - optional text (specified in C++)
        #  - C++ type
        #  - default value
        docs = []
        if doc:
            docs.append(doc.strip())
            docs.append("")
        docs.append("type: " + self.cppType)
        docs.append("default: " + repr(self.default))
        self.doc = "\n".join(docs)

    def get(self, owner):
        if self.name in owner._propertyData:
            return owner._propertyData[self.name]
        else:
            # return a clone of the default
            return type(self.default)(self.default)
            # raise AttributeError(self.name)

    def set(self, owner, value):
        if self._validator(repr(value)):
            owner._propertyData[self.name] = value
        else:
            raise TypeError("Property '%s.%s' cannot accept value %r: %s" %
                            (owner.__class__.__name__, self.name, value, self._validator.__doc__))

    def delete(self, owner):
        if self.name in owner._propertyData:
            del owner._propertyData[self.name]
        # raise RuntimeError("Cannot delete property %s" % self.name)

class DefaultWrapper(object):
    """
    Wrapper for the default value of complex mutable properties, used by
    VectorProperty to correctly implement the "property" semantics for vectors
    and maps.
    """
    def __init__(self, owner, name, value):
        """
        Initialize the wrapper.
        @param owner: instance of the configurable that this list belongs to
        @param name: name of the property in the owner
        @param object: object to wrap (value of the property)
        """
        class MethodWrapper(object):
            """
            Simple wrapper class to implement a simple and generic closure.
            """
            def __init__(self, owner, name, value, method):
                """
                Data for the closure.
                """
                self.owner = owner
                self.name = name
                self.value = value
                self.method = method
            def __call__(self, *args, **kwargs):
                """
                Call the wrapped method and fix the property value in its owner.
                """
                retval = apply(getattr(self.value, self.method), args, kwargs)
                self.owner._propertyData[self.name] = self.value
                return retval
        # wrap the methods that need special treatment
        wrapped = set(["__setitem__", "__setslice__",
                       "__iadd__", "extend",
                       "append", "insert"])
        for method in wrapped:
            if hasattr(value, method):
                setattr(self, method, MethodWrapper(owner, name, value, method))
        # forward the other non-defined methods (forcing __str__ and __repr__
        # because they are always present)
        for method in [ m for m in dir(value) ]:
            if not hasattr(self, method):
                setattr(self, method, getattr(value, method))
        self._value = value
    def __str__(self):
        return str(self._value)
    def __repr__(self):
        return repr(self._value)




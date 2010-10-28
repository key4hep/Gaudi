'''
Created on 28/mag/2010

@author: Marco Clemencic
'''

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

# Default no-op validation function.
_defaultValidator = lambda _: True

class Property(object):
    '''
    Generic property class to handle
    '''
    def __init__(self, name, validator = _defaultValidator,
                 default = None, doc = "Undocumented"):
        '''
        Constructor
        '''
        self.name = name

        self._validator = validator

        if default is None:
            self.default = self._validator.default
        else:
            if self._validator(repr(default)):
                self.default = default
            else:
                raise TypeError("Bad default value %r (%s)" %
                                (default, self._propType))

        self.doc = doc

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
            raise TypeError("Property '%s.%s' cannot accept value %r (%s)" %
                            (owner.__class__.__name__, self.name, value, self._propType))

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




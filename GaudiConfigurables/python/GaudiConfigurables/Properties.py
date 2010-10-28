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

from Validators import defaultValidator as _defaultValidator, ValidatorError

class BaseProperty(object):
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
            self._validator(default) # raise if not good
            self.default = default
        
        self.doc = doc
        
    def get(self, owner):
        if self.name in owner._propertyData:
            return owner._propertyData[self.name]
        else:
            # return a clone of the default
            return type(self.default)(self.default)
            # raise AttributeError(self.name)
    
    def set(self, owner, value):
        try:
            self._validator(value) # raise if not good
            owner._propertyData[self.name] = value
        except ValidatorError, v:
            raise TypeError("Property '%s.%s' cannot accept value %r ('%s' expected)" %
                            (owner.__class__.__name__, self.name, value, v.reqType))

    def delete(self, owner):
        if self.name in owner._propertyData:
            del owner._propertyData[self.name]
        # raise RuntimeError("Cannot delete property %s" % self.name)

class ListPropertyValue(list):
    """
    Class extending the standard list with type checking.
    """
    def __new__(cls, validator = _defaultValidator, *args):
        """
        Needed to be able to pickle correctly.
        """
        instance = super(ListPropertyValue, cls).__new__(cls)
        instance._owner, instance._name = None, None
        instance._validator = validator
        return instance
    def __init__(self, validator = _defaultValidator, *args):
        """
        Initialize the instance.
        
        @param validator: type validator for the elements of the list
        @param initializer: optional iterable to fill the instance
        """
        super(ListPropertyValue, self).__init__()
        # Special data member used to activate the special behavior for defaults
        self._owner, self._name = None, None
        self._validator = validator
        if args:
            if len(args) > 1:
                raise TypeError("%s() takes at most 1 argument (%d given)"
                                % (self.__class__.__name__, len(args)))
            if isinstance(args[0], list):
                # allow to be initialized only from lists
                self.extend(args[0])
            else:
                raise ValidatorError("list", args[0])
    def withOwner(self, owner, name):
        self._owner, self._name = owner, name
        return self
    def __attach__(self):
        if self._owner:
            self._owner._propertyData[self._name] = self
            self._owner = None # once it is attached we do not need to attach it again
    def __setitem__(self, index, value):
        self._validator(value)
        super(ListPropertyValue, self).__setitem__(index, value)
        self.__attach__()
    def __setslice__(self, i, j, seq):
        for v in seq:
            self._validator(v)
        super(ListPropertyValue, self).__setslice__(i, j, seq)
        self.__attach__()
    def append(self, value):
        self._validator(value)
        super(ListPropertyValue, self).append(value)
        self.__attach__()
    def __iadd__(self, seq):
        for v in seq:
            self._validator(v)
        retval = super(ListPropertyValue, self).__iadd__(seq)
        self.__attach__()
        return retval
    def extend(self, seq):
        for v in seq:
            self._validator(v)
        retval = super(ListPropertyValue, self).extend(seq)
        self.__attach__()
        return retval
    def insert(self, index, value):
        self._validator(value)
        super(ListPropertyValue, self).insert(index, value)
        self.__attach__()
    def __getnewargs__(self):
        """
        Needed for correct pickling/unpickling.
        """
        return (self._validator, list(self))

class VectorProperty(BaseProperty):
    """
    The vector property handle properties that must host a vector.
    The validation must be done on each element even if inserted using the
    internal instance (like with ".append()" and "+="). Moreover, the default value
    (which is an unbound copy of the internal default) must be able to set the
    internal storage of the owner if modified.
    For all that we need a clever wrapping around the standard python list.
    """
    def __init__(self, name, validator = _defaultValidator,
                 default = [], doc = "Undocumented"):
        '''
        Constructor
        '''
        # Temporarily ignore the default
        super(VectorProperty, self).__init__(name, validator, None, doc)
        # Override the default
        self.default = ListPropertyValue(validator, default)
    
    def get(self, owner):
        if self.name in owner._propertyData:
            return owner._propertyData[self.name]
        else:
            # return a clone of the default, enabling the special behavior for defaults
            return ListPropertyValue(self._validator, self.default).withOwner(owner, self.name)
            # raise AttributeError(self.name)
    
    def set(self, owner, value):
        try:
            owner._propertyData[self.name] = ListPropertyValue(self._validator, value)
        except ValidatorError, v:
            raise TypeError("Property '%s.%s' cannot accept value %r ('[%s]' expected)" %
                            (owner.__class__.__name__, self.name, value, v.reqType))


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



def Property(name, cppType, default = None, doc = "Undocumented"):
    # handle the special cases of vectors and maps
    pass
    
    

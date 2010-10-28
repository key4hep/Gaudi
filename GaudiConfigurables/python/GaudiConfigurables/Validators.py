'''
Created on 31/mag/2010

@author: Marco Clemencic
'''

class ValidatorError(TypeError):
    """
    Exception class used by the Validator object.
    """
    def __init__(self, reqType, value):
        self.reqType = reqType
        self.value = value
    def __str__(self):
        return "received %r ('%s') while expecting '%s'" % (self.value,
                                                            type(self.value).__name__,
                                                            self.reqType)

class Validator(object):
    """
    Base class for type validator objects.
    """
    def __init__(self, default = None):
        """
        Initialize the instance setting the default value for the type.
        """
        self.default = default
    def __call__(self, value):
        """
        Method used to check if the value is acceptable.
        If it is not valid, it should raise a ValidationError.
        """
        pass
# Instance of the basic validator (used in default arguments)
defaultValidator = Validator()     
    
class CIntValidator(Validator):
    def __init__(self, size, signed = True):
        super(CIntValidator, self).__init__(0)
        self.size = size
        if signed:
            t = 1 << (size-1)
            self._min = -t
            self._max = t - 1
        else:
            t = 1 << size
            self._min = 0
            self._max = t - 1
    def __call__(self, value):
        # is value an integer?
        try:
            if int(value) == value:
                # does it fit?
                if value >= self._min and value <= self._max:
                    return
        except:
            # whatever exception we got, we need to translate it
            pass
        if self._min == 0:
            typename = "uint%d"
        else:
            typename = "int%d"
        typename %= self.size
        raise ValidatorError(typename, value)

class CFPValidator(Validator):
    def __init__(self):
        super(CFPValidator, self).__init__(0)
    def __call__(self, value):
        # is value a number?
        if type(value) in [int, long, float]:
            return
        raise ValidatorError("float", value)

class CTypesValidator(Validator):
    @classmethod
    def _ctypeForName(cls, name):
        import ctypes, re
        name = name.replace("const", "")
        name = re.sub("unsigned *", "u", name)
        name = re.sub("long *int", "long", name)
        name = re.sub("long *long", "longlong", name)
        name = re.sub(" *\*", "_p", name)
        name = "c_" + name.strip()
        return getattr(ctypes, name)

    def __init__(self, typename):
        self._type = CTypesValidator._ctypeForName(typename)
        super(CTypesValidator, self).__init__(self._type().value)
    
    def __call__(self, value):
        try:
            if self._type(value).value == value:
                return
        except:
            pass
        raise ValidatorError(self._type.__name__, value)

class StringValidator(Validator):
    def __init__(self):
        super(StringValidator, self).__init__("")
    def __call__(self, value):
        if type(value) is not str:
            raise ValidatorError("string", value)


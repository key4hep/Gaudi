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
        return True
# Instance of the basic validator (used in default arguments)
defaultValidator = Validator()

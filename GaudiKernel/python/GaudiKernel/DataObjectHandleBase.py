__version__ = "$Revision: 0.1 $"
__doc__ = """The python module holding python bindings to DataObjectHandle"""

# s = "/Path/to/Address"

class DataObjectHandleBase(object):
    
    __slots__ = ('Path' )

    #define accessTypes
    
    def __init__(self, *args):
        object.__init__(self)
        
        if len(args) != 1: raise RuntimeError("must have one argument!")
        self.Path = args[0]
            
    def __str__(self):
        return self.Path
               
    def __repr__(self):
        return "%s(\"%s\")" % (self.__class__.__name__, self.__str__())
    
    def toStringProperty(self):
        return self.__str__()
            

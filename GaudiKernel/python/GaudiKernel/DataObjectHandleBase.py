__version__ = "$Revision: 0.1 $"
__doc__ = """The python module holding python bindings to DataObjectHandle"""

FIELD_SEP = '|'
ITEM_SEP = '#'
ADDR_SEP = '&'

# s = "/Path/to/Address|MODE|OPTIONAL|/alt/path/one&/alt/path/two"

class DataObjectHandleBase(object):
    
    __slots__ = ('Path', 'Mode', 'AlternativePaths', 'Optional')

    #define accessTypes
    READ = 1 << 2
    WRITE = 1 << 4
    UPDATE = READ | WRITE
    
    def __init__(self, *args):
        object.__init__(self)
        
        #if we have one arg: initiliaze from string        
        if len(args) is 1:
            self.fromString(args[0])
        else:
            #else the data is given
            self.fromArray(args)
            
    def fromString(self,s):
        self.fromArray(s.split(FIELD_SEP))
        
    def fromArray(self, a):
        if len(a) == 0: raise RuntimeError("got empty array!")
        self.Path = a[0]
        self.Mode = a[1] if len(a)>1 else self.READ
        self.Optional = a[2] if len(a)>2 else 0
        self.AlternativePaths = [ i for i in a[3].split(ADDR_SEP) if i ] if len(a)>3 else [ ]
        if len(a) > 4: raise RuntimeError("got too long array: %s",a)

                
    def __str__(self):

        s = self.Path + FIELD_SEP + str(int(self.Mode)) + FIELD_SEP
        s += str(int(self.Optional)) + FIELD_SEP
        s += ADDR_SEP.join( self.AlternativePaths )
        
        return s
               
    def __repr__(self):
        return "%s(\"%s\")" % (self.__class__.__name__, self.__str__())
    
    def toStringProperty(self):
        return self.__str__()
            

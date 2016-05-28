__version__ = "$Revision: 0.1 $"
__doc__ = """The python module holding python bindings to DataObjectHandle"""

FIELD_SEP = '|'
ITEM_SEP = '#'
ADDR_SEP = '&'

# s = "/Path/to/Address|MODE|OPTIONAL|/alt/path/one&/alt/path/two"

class DataObjectHandleBase(object):
    
    __slots__ = ('Path', 'Mode', 'AlternativePaths', 'Optional')

    #define accessTypes
    READ = 0
    WRITE = 1
    UPDATE = 2
    
    def __init__(self, *args):
        object.__init__(self)
        
        #if we have one arg: initiliaze from string        
        if len(args) is 1:
            self.fromString(args[0])
        else:
            #else the data is given
            self.fromArray(args)
            
    def fromString(self,s):
        if s == "": return
        
        fields = s.split(FIELD_SEP)
        self.fromArray(fields)
        
    def fromArray(self, a):
        if len(a) != 4: return

        self.Path = a[0]
        self.Mode = a[1]
        self.Optional = a[2]
        self.AlternativePaths = [ i for i in a[3].split(ADDR_SEP) if i ]
                
    def __str__(self):

        s = self.Path + FIELD_SEP + str(int(self.Mode)) + FIELD_SEP
        s += str(int(self.Optional)) + FIELD_SEP
        s += ADDR_SEP.join( self.AlternativePaths )
        
        return s
               
    def __repr__(self):
        print "DOHB:__repr__", self.__class__.__name__
        return "%s(""%s"")" % (self.__class__.__name__, self.__str__())
    
    def toStringProperty(self):
        print "DOHB:toStringProperty"
        return self.__str__()
            

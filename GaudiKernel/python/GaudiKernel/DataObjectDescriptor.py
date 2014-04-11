__version__ = "$Revision: 0.1 $"
__doc__ = """The python module holding python bindings to DataObjectDescriptor"""

FIELD_SEP = '|'
ITEM_SEP = '#'
ADDR_SEP = '&'

# s = "tracks|/evt/rec/tracks|0|0#hits|/evt/velo/hits|0|0#selTracks|/evt/mySel/tracks|0|1"

class DataObjectDescriptor(object):
    
    __slots__ = ('Tag', 'Path', 'AlternativePaths', 'Optional', 'AccessType')

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
        if s == "":
            return
        
        fields = s.split(FIELD_SEP)
        self.fromArray(fields)
        
    def fromArray(self, a):
        if len(a) != 4:
            return
        
        self.Tag = a[0]
        
        if not ADDR_SEP in a[1]: #only one address provided 
            self.Path = a[1]
            self.AlternativePaths = []
        else: #we have alternative addresses
            addr = a[1].split(ADDR_SEP)
            self.Path = addr[0]
            self.AlternativePaths = addr[1:]
        
        self.Optional = int(a[2]) == 1 
        self.AccessType = int(a[3])
        
    def __str__(self):
        s =  self.Tag + FIELD_SEP + self.Path 
        
        for a in self.AlternativePaths:
            s += ADDR_SEP + a
            
        s += FIELD_SEP
        
        s += str(int(self.Optional)) + FIELD_SEP + str(self.AccessType)
        
        return s
               
    def __repr__(self):
        return "%s(""%s"")" % (self.__class__.__name__, self.__str__())
    
    def toStringProperty(self):
        return self.__str__()

class DataObjectDescriptorCollection(object):
    
    def __init__(self,dataItems):        
        object.__init__(self)
        
        #dataItems is a string of format
        # "itemITEM_SEPitemITEM_SEP"
        # each item is a string of 
        # "tagFIELD_SEPaddressFIELD_SEPoptionalFIELD_SEPaccessType"
        
        if dataItems == "":
            return
        
        #import pdb; pdb.set_trace()
        
        items = dataItems.split(ITEM_SEP)
        
        for item in items:
            d = DataObjectDescriptor(item)
            object.__setattr__(self, d.Tag, d)
            
    def __getattr__(self, name):
        #is only called if attribute doesn't exist
        #don't allow creation of new DataItems
        raise AttributeError(name)
    
    def __setattr__(self, name, value):
        if name in self.__dict__:
            return object.__setattr__(self, name, value)
        else:
            #don't allow creation of new attributes
            raise AttributeError(name)
        
    def __str__(self):
        return ITEM_SEP.join([str(self.__dict__[s]) for s in self.__dict__])
    
    def __repr__(self):
        return "%s(""%s"")" % (self.__class__.__name__, self.__str__())
    
    def toStringProperty(self):
        return self.__str__()
        
            
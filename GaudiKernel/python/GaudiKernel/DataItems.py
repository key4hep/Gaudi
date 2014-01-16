__version__ = "$Revision: 0.1 $"
__doc__ = """The python module holding python bindings to DataItems"""

FIELD_SEP = '|';
ITEM_SEP = '#';

# s = "tracks|/evt/rec/tracks|0|0#hits|/evt/velo/hits|0|0#selTracks|/evt/mySel/tracks|0|1"

class DataItem(object):
    
    __slots__ = ('tag', 'productName', 'optional', 'accessType')

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
        
        self.tag = a[0]
        self.productName = a[1]
        self.optional = int(a[2]) == 1 
        self.accessType = int(a[3])
        
    def __str__(self):
        return self.tag + FIELD_SEP + self.productName\
               + FIELD_SEP + str(int(self.optional)) + FIELD_SEP + str(self.accessType)
               
    def __repr__(self):
        return "%s(""%s"")" % (self.__class__.__name__, self.__str__())
    
    def toStringProperty(self):
        return self.__str__()

class DataItems(object):
    
    def __init__(self,dataItems):        
        object.__init__(self)
        
        #dataItems is a string of format
        # "itemITEM_SEPitemITEM_SEP"
        # each item is a string of 
        # "tagFIELD_SEPproductNameFIELD_SEPoptionalFIELD_SEPaccessType"
        
        if dataItems == "":
            return
        
        #import pdb; pdb.set_trace()
        
        items = dataItems.split(ITEM_SEP)
        
        for item in items:
            d = DataItem(item)
            object.__setattr__(self, d.tag, d)
            
    def __getattr__(self, name):
        #is only called if attribute doesn't exist
        #don't allow creation of new DataItems
        raise AttributeError
    
    def __setattr__(self, name, value):
        if name in self.__dict__:
            return object.__setattr__(self, name, value)
        else:
            #don't allow creation of new attributes
            raise AttributeError
        
    def __str__(self):
        return ITEM_SEP.join([str(self.__dict__[s]) for s in self.__dict__])
    
    def __repr__(self):
        return "%s(""%s"")" % (self.__class__.__name__, self.__str__())
    
    def toStringProperty(self):
        return self.__str__()
        
            
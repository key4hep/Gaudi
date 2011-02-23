# File: GaudiPython/Pythonizations.py
# Author: Pere Mato (pere.mato@cern.ch)

""" This Pythonizations module provides a number of useful pythonizations
    of adaptation of some classes.
"""

__all__ = [ ]

import PyCintex
gbl = PyCintex.gbl

if not hasattr(gbl,'ostream') : gbl.gROOT.ProcessLine("#include <ostream>")
if not hasattr(gbl,'stringstream') : gbl.gROOT.ProcessLine("#include <sstream>")

#--- Hack to match the name scheme of dictionary on Linux ----------------------------
_loadDict_save = PyCintex.loadDict
def _loadDict(name):
    import sys
    if sys.platform != 'win32' and name[:3] != 'lib' : name = 'lib'+name
    return _loadDict_save(name)
PyCintex.loadDict = _loadDict

#--- Adding extra functionality to C++ raw classes------------------------------------
def _printHisto1D(h) :
    x = h.axis()
    return  'Histogram 1D "%s" %d bins [%f,%f]' % (h.title(), x.bins(), x.lowerEdge(), x.upperEdge())
def _contentsHisto1D(h) :
    x = h.axis()
    return  map(h.binEntries, range(x.bins()))
def _printHisto2D(h) :
    x,y = h.xAxis(),h.yAxis()
    return  'Histogram 2D "%s" %d xbins [%f,%f], %d ybins [%f,%f]' % \
    (h.title(), x.bins(), x.lowerEdge(), x.upperEdge(), y.bins(), y.lowerEdge(), y.upperEdge() )
def _printStatusCode(s) :
    if s.isSuccess() : return 'SUCCESS'
    else             : return 'FAILURE'
def _printBitReference(b) :
    return str(1==b.bool())
def _printFillStream(o) :
    if  o :
        s = gbl.stringstream()
        o.fillStream(s)
        out = s.str()
        if out == '' :
            out = o.__class__.__name__ + ' object'
            if hasattr( o, 'hasKey') and o.hasKey() :
                out += ' key = '+ str(o.key())
    else :
        out = o.__class__.__name__ + ' NULL object'
    return out
def _container__getitem__(self, k) :
    return self.containedObject(k)
def _container__len__(self) :
    return self.numberOfObjects()
def _container__iter__(self) :
    if hasattr(self,'containedObjects') : sequential = self.containedObjects()
    else                                : sequential = self
    count = 0
    limit = self.__len__()
    while count < limit :
        yield sequential.__getitem__(count)
        count += 1

def _draw_aida_ ( self , *args ) :
    """
    Draw AIDA histogram (through access to internal ROOT histogram

    >>> aida = ...    # get the historgam
    >>> aida.Draw()

    """
    _fun = PyCintex.gbl.Gaudi.Utils.Aida2ROOT.aida2root
    _root = _fun ( self )
    return _root.Draw( *args )

gbl.AIDA.IHistogram1D.__repr__ = _printHisto1D
gbl.AIDA.IHistogram1D.contents = _contentsHisto1D
gbl.AIDA.IHistogram2D.__repr__ = _printHisto2D
for h in (  gbl.AIDA.IHistogram   ,
            gbl.AIDA.IHistogram1D ,
            gbl.AIDA.IHistogram2D ,
            gbl.AIDA.IHistogram3D ,
            gbl.AIDA.IProfile1D   ,
            gbl.AIDA.IProfile2D   ) :
    h.Draw = _draw_aida_
    h.plot = _draw_aida_

gbl.StatusCode.__repr__ = _printStatusCode
try: gbl._Bit_reference.__repr__ = _printBitReference
except: pass
gbl.ContainedObject.__repr__ = _printFillStream
gbl.DataObject.__repr__ = _printFillStream
gbl.ObjectContainerBase.__getitem__ = _container__getitem__
gbl.ObjectContainerBase.__len__ = _container__len__
gbl.ObjectContainerBase.__iter__ = _container__iter__

gbl.IUpdateManagerSvc.update = lambda self,obj: gbl.IUpdateManagerSvc.PythonHelper.update(self,obj)
gbl.IUpdateManagerSvc.invalidate = lambda self,obj: gbl.IUpdateManagerSvc.PythonHelper.invalidate(self,obj)

#---Globals--------------------------------------------------------------------
gbl.StatusCode.SUCCESS = 1
gbl.StatusCode.FAILURE = 0

# - string key, equality
if hasattr ( gbl.Gaudi.StringKey ,'__cpp_eq__' ) :
    _eq = gbl.Gaudi.StringKey.__cpp_eq__
    setattr ( gbl.Gaudi.StringKey ,'__eq__' , _eq )

# - string key, non-equality
if hasattr ( gbl.Gaudi.StringKey ,'__cpp_ne__' ) :
    _ne = gbl.Gaudi.StringKey.__cpp_ne__
    setattr ( gbl.Gaudi.StringKey ,'__ne__' , _ne )


#---Enabling Pickle support----------------------------------------------------
if  gbl.gROOT.GetVersionInt() <= 51800 :
    import libPyROOT
    gbl.GaudiPython.PyROOTPickle.Initialize(libPyROOT, libPyROOT.ObjectProxy)

# =============================================================================
## decorate some map-like objects 
# =============================================================================
## The iterator for MapBase class
#
#  @code
#
#    >>> m = ...  ## the map
#    >>> for key in m : print key , m[key]
#    
#  @endcode 
#  @see Gaudi::Utils::MapBase
#  @see GaudiUtils::Map
#  @see GaudiUtils::HashMap
#  @see GaudiUtils::VectorMap
#  @see GaudiUtils::Map::key_at
#  @see GaudiUtils::HashMap::key_at
#  @see GaudiUtils::VectorMap::key_at
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date 2010-02-20
def __mapbase_iter__ ( self ) :
    """
    The iterator for MapBase-based containers
    
    >>> m = ...  ## the map
    >>> for key in m : print key , m[key]
    
    """
    _size  = len ( self )
    _index = 0
    while _index < _size :
        yield self.key_at ( _index )
        _index +=1

# =============================================================================
## The iterator for MapBase class
#
#  @code
#
#    >>> m = ...  ## the map
#    >>> for key,value in m.iteritems() : print key , value 
#    
#  @endcode 
#  @see Gaudi::Utils::MapBase
#  @see GaudiUtils::Map
#  @see GaudiUtils::HashMap
#  @see GaudiUtils::VectorMap
#  @see GaudiUtils::Map::key_at
#  @see GaudiUtils::HashMap::key_at
#  @see GaudiUtils::VectorMap::key_at
#  @see GaudiUtils::Map::value_at
#  @see GaudiUtils::HashMap::value_at
#  @see GaudiUtils::VectorMap::value_at
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date 2010-02-20
def __mapbase_iteritems__ ( self ) :
    """
    The iterator for MapBase-based containers
    
    >>> m = ...  ## the map
    >>> for key,value in m.iteritems() : print key, value
    
    """
    _size  = len ( self )
    _index = 0
    while _index < _size :
        _key = self.key_at  ( _index ) 
        yield ( _key , self.at ( _key ) ) 
        _index +=1
        
# ============================================
## Get the list of keys for the map
#
#  @code
#
#    >>> m    = ...        ## the map
#    >>> keys = m.keys()   ## get the list of keys
#
#  @endcode
#  @see Gaudi::Utils::MapBase
#  @see GaudiUtils::Map
#  @see GaudiUtils::HashMap
#  @see GaudiUtils::VectorMap
#  @see GaudiUtils::Map::key_at
#  @see GaudiUtils::HashMap::key_at
#  @ see GaudiUtils::VectorMap::key_at
#  @author Vanya BELYAEV Ivan.BElyaev@itep.ru
#  @date 2010-02-20
def __mapbase_keys__ ( self ) :
    """
    Get the list of keys 
    
    >>> m = ...           ## the map
    >>> keys = m.keys()   ## get the list of keys 
    
    """
    _size  = len ( self )
    _keys  = [] 
    for i in range ( 0 , _size ) : _keys.append ( self.key_at ( i ) ) 
    return _keys

# ============================================
## Get the list of items for the map
#
#  @code
#
#    >>> m     = ...        ## the map
#    >>> items = m.items()   ## get the list of items
#
#  @endcode
#  @see Gaudi::Utils::MapBase
#  @see GaudiUtils::Map
#  @see GaudiUtils::HashMap
#  @see GaudiUtils::VectorMap
#  @see GaudiUtils::Map::key_at
#  @see GaudiUtils::HashMap::key_at
#  @ see GaudiUtils::VectorMap::key_at
#  @author Vanya BELYAEV Ivan.BElyaev@itep.ru
#  @date 2010-02-20
def __mapbase_items__ ( self ) :
    """
    Get the list of items
    
    >>> m     = ...        ## the map
    >>> items = m.keys()   ## get the list of items
    
    """
    _size  = len ( self )
    _items = []
    for i in range ( 0 , _size )  : 
        _key   = self.key_at   ( i     )
        _value = self.at       ( _key  )  
        _items.append ( ( _key , _value ) ) 
    return _items

# ============================================
## Get the list of values for the map
#
#  @code
#
#    >>> m      = ...           ## the map
#    >>> values = m.values ()   ## get the list of values
#
#  @endcode
#  @see Gaudi::Utils::MapBase
#  @see GaudiUtils::Map
#  @see GaudiUtils::HashMap
#  @see GaudiUtils::VectorMap
#  @see GaudiUtils::Map::value_at
#  @see GaudiUtils::HashMap::value_at
#  @ see GaudiUtils::VectorMap::value_at
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date 2010-02-20
def __mapbase_values__ ( self ) :
    """
    Get the list of values
    
    >>> m      = ...          ## the map
    >>> values = m.values()   ## get the list of values
    
    """
    _size   = len ( self )
    _values = []
    for i in range ( 0 , _size ) : 
        _value = self.value_at ( i ) 
        _values.append ( _value ) 
    return _values

# ============================================
## Check if the certain key is in the map 
#
#  @code
#
#    >>> m      = ...        ## the map
#    >>> if 'a' in m : print 'key is in the map!'
#
#  @endcode
#  @see Gaudi::Utils::MapBase
#  @see GaudiUtils::Map
#  @see GaudiUtils::HashMap
#  @see GaudiUtils::VectorMap
#  @see GaudiUtils::Map::count
#  @see GaudiUtils::HashMap::count
#  @ see GaudiUtils::VectorMap::count 
#  @author Vanya BELYAEV Ivan.BElyaev@itep.ru
#  @date 2010-02-20
def __mapbase_contains__ ( self , key ) :
    """
    Check if the certainkey is in the map
    
    >>> m     = ...        ## the map
    >>> if 'a' in m : ...  ##  chekc the presence of the key in the map
    
    """
    _num = self.count ( key )
    return False if 0 == _num else True 

# ============================================
## Get the value for certain key,
#   return predefined value otherwise 
#
#  @code
#
#    >>> m      = ...          ## the map
#    >>> v = m.get( key , 15 ) ## return the value[key] for existing key, else 15
#
#  @endcode
#  @see Gaudi::Utils::MapBase
#  @see GaudiUtils::Map
#  @see GaudiUtils::HashMap
#  @see GaudiUtils::VectorMap
#  @see GaudiUtils::Map::count
#  @see GaudiUtils::HashMap::count
#  @ see GaudiUtils::VectorMap::count 
#  @author Vanya BELYAEV Ivan.BElyaev@itep.ru
#  @date 2010-02-20
def __mapbase_get__ ( self , key , value = None ) :
    """
    Get the value for the certain key, or 'value' otherwise 
    
    >>> m     = ...        ## the map
    >>> v = m.get ( key , 15 ) 
    
    """
    if key in self : return self.at( key ) 
    return value 

# ============================================
## Representation of MapBase-based maps 
#
#  @code
#
#    >>> m      = ...        ## the map
#    >>> print m 
#
#  @endcode
#  @see Gaudi::Utils::MapBase
#  @see GaudiUtils::Map
#  @see GaudiUtils::HashMap
#  @see GaudiUtils::VectorMap
#  @author Vanya BELYAEV Ivan.BElyaev@itep.ru
#  @date 2010-02-20
def __mapbase_str__ ( self  ) :
    """
    Representation of MapBase-based maps:
    
    >>> m     = ...        ## the map
    >>> print map 
    
    """
    _result  = ' { '
    _size  = len ( self )
    for i in range ( 0 , _size ) :
        _key = self.key_at   ( i    )
        _val = self.at       ( _key )  
        if 0 != i : _result += ' , '
        _result += " %s : %s " % ( str ( _key ) , str ( _val ) )
    _result += ' } '
    return _result 

# ============================================
## "Setitem" for MapBase-based maps:
#
#  @code
#
#    >>> m        = ...        ## the map
#    >>> m [ key] = value    ## set the item 
#
#  @endcode
#  @see Gaudi::Utils::MapBase
#  @see GaudiUtils::Map
#  @see GaudiUtils::HashMap
#  @see GaudiUtils::VectorMap
#  @see GaudiUtils::Map::update
#  @see GaudiUtils::HashMap::update
#  @see GaudiUtils::VectorMap::update
#  @author Vanya BELYAEV Ivan.BElyaev@itep.ru
#  @date 2010-02-20
def __mapbase_setitem__ ( self , key , value ) :
    """
    'Set-item' for MapBase-based maps:
    
    >>> m      = ...        ## the map
    >>> m[key] = value     ## set the item 
    
    """
    _replaced = True if key in self else False 
    self.update ( key , value )
    return _replaced

# ============================================
## "Del-item" for MapBase-based maps:
#
#  @code
#
#    >>> m        = ...   ## the map
#    >>> del m [ key]     ## del th eitem 
#
#  @endcode
#
#  @see Gaudi::Utils::MapBase
#  @see GaudiUtils::Map
#  @see GaudiUtils::HashMap
#  @see GaudiUtils::VectorMap
#  @see GaudiUtils::Map::erase
#  @see GaudiUtils::HashMap::erase
#  @see GaudiUtils::VectorMap::erase
#  @author Vanya BELYAEV Ivan.BElyaev@itep.ru
#  @date 2010-02-20
def __mapbase_delitem__ ( self , key ) :
    """
    'Del-item' for MapBase-based maps:
    
    >>> m      = ...        ## the map
    >>> del m[key] 
    
    """
    _erased = True if key in self else False 
    self.erase ( key ) 
    return _erased

gbl.Gaudi.Utils.MapBase . __len__       = lambda s   : s.size()
gbl.Gaudi.Utils.MapBase . __iter__      = __mapbase_iter__ 
gbl.Gaudi.Utils.MapBase .   keys        = __mapbase_keys__ 
gbl.Gaudi.Utils.MapBase . __iteritems__ = __mapbase_iteritems__ 
gbl.Gaudi.Utils.MapBase .   iteritems   = __mapbase_iteritems__ 
gbl.Gaudi.Utils.MapBase .   items       = __mapbase_items__ 
gbl.Gaudi.Utils.MapBase .   values      = __mapbase_values__
gbl.Gaudi.Utils.MapBase . __contains__  = __mapbase_contains__ 
gbl.Gaudi.Utils.MapBase .   has_key     = __mapbase_contains__ 
gbl.Gaudi.Utils.MapBase .   get         = __mapbase_get__      
gbl.Gaudi.Utils.MapBase . __str__       = __mapbase_str__     
gbl.Gaudi.Utils.MapBase . __repr__      = __mapbase_str__ 
gbl.Gaudi.Utils.MapBase . __setitem__   = __mapbase_setitem__ 
gbl.Gaudi.Utils.MapBase . __delitem__   = __mapbase_delitem__ 
gbl.Gaudi.Utils.MapBase . __getitem__   = lambda s,key : s.at ( key )  

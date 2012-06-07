#!/usr/bin/env python
# =============================================================================
# $Id: GaudiAlgs.py,v 1.2 2008/07/15 16:05:53 marcocle Exp $
# =============================================================================
## @file
#
# Helper module, which effectively 'imports' few useful C++ algorithmic
# base classes into Python
#
#
#              The major imported classes are :
#
# - GaudiAlgo - analogue for GaudiAlgorithm C++ class from GaudiAlg package
# - HistoAlgo - analogue for GaudiHistoAlg  C++ class from GaudiAlg package
# - TupleAlgo - analogue for GaudiTupleAlg  C++ class from GaudiAlg package
#
# @author Vanya BELYAEV ibelyaev@physics.syr.edu
# @date 2006-11-26
# =============================================================================
"""
*******************************************************************************
*                                                * 'Physisics do not like it, *
*                                                *  physisics do not need it, *
*                                                *  physisics do not use  it' *
*                                                * ****************************
*                                                                             *
* Helper module, which effectively 'imports' few useful C++ algorithmic       *
* base classes into Python                                                    *
*                                                                             *
*******************************************************************************
*              The major imported classes are :                               *
*                                                                             *
* (1) GaudiAlgo - analogue for GaudiAlgorithm C++ class from GaudiAlg package *
* (2) HistoAlgo - analogue for GaudiHistoAlg  C++ class from GaudiAlg package *
* (3) TupleAlgo - analogue for GaudiTupleAlg  C++ class from GaudiAlg package *
*******************************************************************************
"""
# =============================================================================
__author__ = 'Vanya BELYAEV  Ivan.Belyaev@lapp.in2p3.fr'
# =============================================================================
# list of "public" symbols
# =============================================================================
__all__ = (
    'GaudiAlgo' ,                    ##               base class for algorithms
    'HistoAlgo' ,                    ## base class for histo-related algorithms
    'TupleAlgo' ,                    ## base class for tuple-related algorithms
    'Tuple'     ,                    ##                                 N-Tuple
    'HistoID'   ,                    ##                         ID for N-tuples
    'TupleID'   ,                    ##                       ID for Histograms
    'aida2root' ,                    ##                  AIDA -> ROOT converter
    'SUCCESS'                        ##                             status code
    )
# =============================================================================
# import core of Gaudi
import GaudiPython.Bindings                           ##       The basic module
iAlgorithm = GaudiPython.Bindings.iAlgorithm          ##    Algorithm interface
iAlgTool   = GaudiPython.Bindings.iAlgTool            ##         Tool interface
#
from GaudiPython.Bindings import (
    SUCCESS        ,                                  ##            status code
    InterfaceCast  ,                                  ##       "queryInterface"
    iDataSvc       ,                                  ##           Data Service
    iHistogramSvc  ,                                  ##      Histogram Service
    iNTupleSvc     ,                                  ##        N-Tuple service
    AppMgr                                            ##    Application Manager
    )
#
from GaudiPython.Bindings   import gbl as cpp         ##  global C++ namepspace
from GaudiPython.HistoUtils import aida2root          ## AIDA -> ROTO converter
# =============================================================================
# std C++ namespace
std = cpp.std                                         ## std C++ namespace

## "typedef" for GaudiPython::Vector
Vector = std.vector('double')
## "typedef" for GaudiPython::Matrix
Matrix = std.vector('std::vector<double>')

## histogram and N-Tuple universal identifier
HID       = cpp.GaudiAlg.ID
HistoID   = HID
TID       = HID
TupleID   = TID

## get the decorator:
AlgDecorator      = cpp.GaudiPython.AlgDecorator
HistoDecorator    = cpp.GaudiPython.HistoDecorator
TupleAlgDecorator = cpp.GaudiPython.TupleAlgDecorator
TupleDecorator    = cpp.GaudiPython.TupleDecorator

# =============================================================================
## Useful method to locate the tool a certain
#
#  Usage:
#
#  @code
#
#      # locate public tool
#      t1 = self.tool(ITrExtrapolator,'TrParabolicExtrapolator')
#      # locate private tool
#      t2 = self.tool(ITrExtrapolator,'TrParabolicExtrapolator',parent=self)
#      # locate public tool with defined name
#      t3 = self.tool(ITrExtrapolator,'TrParabolicExtrapolator/MyExt1')
#      # locate private tool with defined name
#      t4 = self.tool(ITrExtrapolator,'TrParabolicExtrapolator/MyExt2',parent=self)
#      # locate public tool with defined name
#      t5 = self.tool(ITrExtrapolator,'TrParabolicExtrapolator','MyExt3')
#      # locate private tool with defined name
#      t6 = self.tool(ITrExtrapolator,'TrParabolicExtrapolator','MyExt4',parent=self)
#
#  @endcode
#
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
def _tool_ ( self             ,
             interface        ,
             typename         ,
             name     = None  ,
             parent   = None  ,
             create   = True  ) :
    """
    Useful method to locate the tool a certain

    Usage:

    # locate public tool
    t1 = self.tool(ITrExtrapolator,'TrParabolicExtrapolator')
    # locate private tool
    t2 = self.tool(ITrExtrapolator,'TrParabolicExtrapolator',parent=self)
    # locate public tool with defined name
    t3 = self.tool(ITrExtrapolator,'TrParabolicExtrapolator/MyExt1')
    # locate private tool with defined name
    t4 = self.tool(ITrExtrapolator,'TrParabolicExtrapolator/MyExt2',parent=self)
    # locate public tool with defined name
    t5 = self.tool(ITrExtrapolator,'TrParabolicExtrapolator','MyExt3')
    # locate private tool with defined name
    t6 = self.tool(ITrExtrapolator,'TrParabolicExtrapolator','MyExt4',parent=self)

    """
    if not interface : interface = cpp.IAlgTool
    if not parent    : parent    = self
    if name          : typename  += '/' + name
    _tool = AlgDecorator.tool_( self , typename , parent , create )
    if not _tool : return None
    _tool = InterfaceCast(interface)(_tool)
    if not _tool :
        self.Warning('Invalid cast to interface %s' % interface )
        return None
    return _tool

# =============================================================================
## Useful method to locate a service:
#
#     Usage:
#
#  @code
#
#     ntsvc = self.svc( INTupleSvc , 'NTUpleSvc' )
#
#  @endcode
#
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
def _service_ ( self            ,
                interface       ,
                name            ,
                create   = True ) :
    """
    Useful method to locate a service:

    Usage:

    ntsvc = self.svc( INTupleSvc , 'NTUpleSvc' )

    """
    if not interface : interface = cpp.IInterface
    _svc = AlgDecorator.svc_ ( self , name , create )
    if not _svc : return None
    _svc = InterfaceCast(interface)(_svc)
    if not _svc :
        self.Warning('Invalid cast to interface %s' % interface )
        return None
    return _svc

# =============================================================================
## The constructor from unique algorithm instance name,
#
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
def _init_ ( self , name , **args ) :
    """
    The constructor from unique algorithm instance name & parameters
    """
    self._Base.__init__( self , self , name )
    appMgr = AppMgr()
    algMgr = appMgr._algmgr
    status = algMgr.addAlgorithm( self )
    if status.isFailure() :
        raise RuntimeError, 'Unable to add Algorithm "' + name + '"'
    iAlgorithm.__init__ ( self , name , self )
    for key in args : setattr ( self , key , args[key] )
    # take some care about the ownership of the algorithms
    if not appMgr.__dict__.has_key ( 'GaudiPythonAlgos') :
        appMgr.__dict__[ 'GaudiPythonAlgos'] = []
    appMgr.__dict__[ 'GaudiPythonAlgos'].append( self ) 

# =============================================================================
## The default initialization (initialization of base C++ class + data
#
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
def _initialize_ ( self ) :
    """
    The default initialization (initialization of base C++ class + data)
    """
    status = self._Base.initialize_( self )
    if status.isFailure() : return status

    # set the basic services
    _e = self._Base.evtSvc( self )
    _s = InterfaceCast(cpp.IService)(_e)
    self._evtSvc_ = iDataSvc ( _s.name() , _e )

    _d = self._Base.detSvc( self )
    _s = InterfaceCast(cpp.IService)(_d)
    self._detSvc_ = iDataSvc ( _s.name() , _d )

    return status

# =============================================================================
## The default initialization (initialization of base C++ class + data members)
#
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
def _initialize_histo_ ( self ) :
    """
    The default initialization (initialization of base C++ class + data members)
    """
    status = _initialize_( self )
    if status.isFailure() : return status

    # set the basic services
    _h = self._Base.histoSvc( self )
    _s = InterfaceCast(cpp.IService)(_h)
    self._histoSvc_ = iHistogramSvc ( _s.name() , _h )

    return status

# =============================================================================
## The default initialization (initialization of base C++ class + data members)
#
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
def _initialize_tuple_ ( self ) :
    """
    The default initialization (initialization of base C++ class + data members)
    """
    status = _initialize_histo_( self )
    if status.isFailure() : return status

    # set the basic services
    if self.produceNTuples() :
        _n = self._Base.ntupleSvc( self )
        _s = InterfaceCast(cpp.IService)(_n)
        self._ntupleSvc_ = iNTupleSvc ( _s.name() , _n )

    if self.produceEvtCols() :
        _n = self._Base.evtColSvc( self )
        _s = InterfaceCast(cpp.IService)(_n)
        self._evtcolSvc_ = iNTupleSvc ( _s.name() , _n )

    return status

# =============================================================================
## Trivial helper function to access Event Data and Event Data Service
#
#    Usage:
#
#  @code
#
#    # get event data service
#    svc = self.evtSvc()
#
#    # get the data
#    hits = self.evtSvc('MC/Calo/Hits')
#
#  @endcode
#
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
def _evtSvc   ( self , location = None ) :
    """
    Trivial helper function to access Event Data and Event Data Service

    Usage:

    # get event data service
    svc = self.evtSvc()

    # get the data
    hits = self.evtSvc('MC/Calo/Hits')
    """
    if not location :
        return self._evtSvc_
    return self._evtSvc_[location]

# =============================================================================
## Trivial helper function to access Detector Data and Detector  Data Service
#
#    Usage:
#
#  @code
#
#    # get detector data service
#    svc = self.detSvc()
#
#    # get the data
#    lhcb = self.detSvc('/dd/Structure/LHCb')
#
#  @endcode
#
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
def _detSvc   ( self ) :
    """
    Trivial helper function to access Detector Data and Event Data Service

    Usage:
    # get detector data service
    svc = self.detSvc()

    # get the data
    lhcb = self.detSvc('/dd/Structure/LHCb')
    """
    if not location :
        return self._detSvc_
    return self._detSvc_[location]

# =============================================================================
## Trivial helper function to access Histogram  Data and Histogram  Data Service
#
#    Usage:
#
#  @code
#
#    # get histogram data service
#    svc = self.histoSvc()
#
#    # get the data
#    histo = self.histoSvc('/stat/Calo/1')
#
#  @endcode
#
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
def _histoSvc  ( self , address = None ) :
    """
    Trivial helper function to access Histogram  Data and Histogram  Data Service

    Usage:

    # get histogram data service
    svc = self.histoSvc()

    # get the data
    histo = self.histoSvc('/stat/Calo/1')
    """
    if not address : return self._histoSvc_
    return self._histoSvc_[ address ]

# =============================================================================
## Trivial function to access the data in TES
def _get      ( self , location ) :
    """
    Trivial function to access the data in TES using the data service
    """
    return self._evtSvc_[location]

# =============================================================================
## Trivial function to access the data in TDS
def _getDet   ( self , location ) :
    """
    Trivial function to access the data in TDS using data service
    """
    return self._detSvc_[location]

# =============================================================================
##  get the data from TES using GaudiCommon methods, respecting RootInTES 
def _get_  ( self , location , rootInTES = True ) :
    """
    Get the object from Transient Event Store using GaudiCommon machinery,
    respecting RootInTES behaviour 
    """
    return AlgDecorator.get_ ( self , location  , rootInTES )
# =============================================================================
##  check the data from TES using GaudiCommon methods, respecting RootInTES 
def _exist_ ( self , location , rootInTES = True ) :
    """
    Check  the object in Transient Event Store using GaudiCommon machinery,
    respecting RootInTES behaviour 
    """
    return AlgDecorator.exist_ ( self , location  , rootInTES )

# =============================================================================
## Trivial helper function to access NTuple Service
def _ntupleSvc ( self ) :
    """
    Trivial function to access N-Tuple Service
    """
    return self._ntupleSvc_

# =============================================================================
## Trivial helper function to access Event Collection Service
def _evtcolSvc ( self ) :
    """
    Trivial function to access Event Collection Service
    """
    return self._evtcolSvc_


# =============================================================================
## The default finalization (finalization of base C++ class)
def _finalize_   ( self ) :
    """
    The default finalization : finalize the base C++ class
    """
    status = self._Base.finalize_  ( self )
    return status
# =============================================================================
## Dummy method returning success
def _success_ ( self ) : return SUCCESS


# =============================================================================
## check the existence of the property with the given name
def _hasProperty_ ( self , pname ) :
    """
    The trivial function which checks the existence of the property with given name
    """
    return cpp.Gaudi.Utils.hasProperty ( self , pname )

# =============================================================================
## get the value of the given property
def _getProperty_ ( self , pname ) :
    """
    Get the property by name
    """
    if not self.hasProperty( pname ) :
        raise AttributeError, 'property %s does not exist' % pname
    return iAlgorithm.__getattr__( self , pname )

# =============================================================================
## set the value for the given property
def _setProperty_ ( self , pname , pvalue ) :
    """
    Set the property from the value
    """
    if not self.hasProperty( pname ) :
        raise AttributeError, 'property %s does not exist' % pname
    return iAlgorithm.__setattr__ ( self , pname , pvalue )

# =============================================================================
## get the attribute or property
def _get_attr_ ( self , pname ) :
    """
    Get the attribute (or property)
    - if the attribute name corresponds to the property name, property value is returned
    """
    if self.hasProperty( pname ) :
        return iAlgorithm.__getattr__ ( self , pname )
    raise AttributeError, 'attribute/property %s does not exist' % pname

# =============================================================================
## set the attribute or property
def _set_attr_ ( self , pname , pvalue ) :
    """
    Set the attribute (or property) :
    - if the attribute name corresponds to the property name, the property is updated
    """
    if not self.hasProperty( pname ) : self.__dict__[pname] = pvalue
    else : iAlgorithm.__setattr__ ( self , pname , pvalue )


_GaudiAlgorithm = cpp.GaudiPython.PyAlg( 'GaudiAlgorithm' )
_GaudiHistoAlg  = cpp.GaudiPython.PyAlg( 'GaudiHistoAlg'  )
_GaudiTupleAlg  = cpp.GaudiPython.PyAlg( 'GaudiTupleAlg'  )

# =============================================================================
## @class GaudiAlgo
#  the base class for all algorithm
#  Python-image of C++ clkass GaudiAlgorithm
#
#  Usage:
#
#  @code
#
#  from GauidPython.GaudiAlgs   import GaudiAlgo, SUCCESS
#
#  class MyClass(GaudiAlgo) :
#       """
#       My specific Algorithm, derived from GaudiAlgo base class
#       """
#       def __init__( self , name , **args ) :
#           """
#           Constructor from algorithm instance name & parameters'
#           """
#           #invoke the constructor of base class
#           GaudiAlgo.__init__(self , name , **args )
#
#       def initialize ( self ) :
#           'Algorithm initialization'
#           # initialize the base class
#           status = GaudiAlgo.initialize( self )
#           if status.isFailure() : return status
#
#           # locate the services and tools
#
#           # locate some tool:
#           extrapolator = self.tool(ITrExtrapolator,'TrExtrapolator')
#
#           # locate the service
#           rndmSvc = self.svc(IRndmGenSvc, 'RndmGenSvc')
#
#           return SUCCESS
#
#
#       def execute ( self ) :
#            'Major method (from IAlgorithm interface)'
#
#           # get some data from Transient Event Store
#           tracks = self.get('/Event/Rec/Tracks')
#
#           # use counters
#           c1 = self.counter('#Tracks')
#           c2 = self.counter('No Tracks')
#           if tracks.empty :
#              c2+=1
#           c1 += tracks->size()
#
#           if 1000 < tracks.size() :
#                return self.Error('The event is *VERY* busy')
#
#           return SUCCESS
#
#  @endcode
#
#  @see GaudiAlgorithm
#
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
class GaudiAlgo ( _GaudiAlgorithm , iAlgorithm ) :
    """
*******************************************************************************
*                                                * 'Physisics do not like it, *
*                                                *  physisics do not need it, *
*                                                *  physisics do not use  it' *
*                                                * ****************************
*  Usage:                                                                     *
*                                                                             *
*  from GaudiPython.GaudiAlgs   import GaudiAlgo, SUCCESS                     *
*                                                                             *
*  class MyClass(GaudiAlgo) :                                                 *
*       ' My specific Algorithm, derived from GaudiAlgo base class '          *
*       def __init__( self , name , **args ) :                                *
*            'Constructor from algorithm instance name & parameters'          *
*             #invoke the constructor of base class                           *
*             GaudiAlgo.__init__(self , name , **args )                       *
*                                                                             *
*       def initialize ( self ) :                                             *
*           'Algorithm initialization'                                        *
*           # initialize the base class                                       *
*           status = GaudiAlgo.initialize( self )                             *
*           if status.isFailure() : return status                             *
*                                                                             *
*           # locate the services and tools                                   *
*                                                                             *
*           # locate some tool:                                               *
*           extrapolator = self.tool(ITrExtrapolator,'TrExtrapolator')        *
*                                                                             *
*           # locate the service                                              *
*           rndmSvc = self.svc(IRndmGenSvc, 'RndmGenSvc')                     *
*                                                                             *
*           return SUCCESS                                                    *
*                                                                             *
*                                                                             *
*       def execute ( self ) :                                                *
*            'Major method (from IAlgorithm interface)'                       *
*                                                                             *
*           # get some data from Transient Event Store                        *
*           tracks = self.get('/Event/Rec/Tracks')                            *
*                                                                             *
*           # use counters                                                    *
*           c1 = self.counter('#Tracks')                                      *
*           c2 = self.counter('No Tracks')                                    *
*           if tracks.empty :                                                 *
*              c2+=1                                                          *
*           c1 += tracks->size()                                              *
*                                                                             *
*           if 1000 < tracks.size() :                                         *
*                return self.Error('The event is *VERY* busy')                *
*                                                                             *
*           return SUCCESS                                                    *
*                                                                             *
*******************************************************************************
    """
    pass

# =============================================================================
## @class HistoAlgo
#  The base class for easy histogramming
#
#  Usage:
#
#
#  @code
#
#  from GaudiPython.GaudiAlgs import HistoAlgo, SUCCESS
#
#  class MyClass(HistoAlgo) :
#       ' My specific Algorithm, derived from GaudiAlgo base class '
#       def __init__( self , name , **args ) :
#            'Constructor from algorithm instance name & parameters'
#             #invoke the constructor of base class
#             HistoAlgo.__init__(self , name , **args )
#
#       def execute ( self ) :
#            'Major method (from IAlgorithm interface)'
#
#           # get some data from Transient Event Store
#           tracks = self.get('/Event/Rec/Tracks')
#
#           self.plot1D ( tracks->size() , '#tracks' , 0 , 100 )
#
#           return SUCCESS
#
#  @endcode
#
# Alternatively the histogram  could be booked in advance:
#
#  @code
#
#  class MyClass(HistoAlgo) :
#       ' My specific Algorithm, derived from GaudiAlgo base class '
#       def __init__( self , name ) :
#            'Constructor from algorithm instance name'
#             #invoke the constructor of base class
#             HistoAlgo.__init__(self , name )
#
#       def initialize ( self ) :
#           'Algorithm initialization'
#           # initialize the base class
#           status = HistoAlgo.initialize( self )
#           if status.isFailure() : return status
#
#           # book the histogram
#           self.h1 = selff.book1D ( '#tracks' , 0 , 100 )
#
#           return SUCCESS
#
#
#       def execute ( self ) :
#            'Major method (from IAlgorithm interface)'
#
#           # get some data from Transient Event Store
#           tracks = self.get('/Event/Rec/Tracks')
#
#           # fill the histogram
#           self.h1.fill ( tracks->size() )
#
#            return SUCCESS
#  @endcode
#
#  @see GaudiHistoAlg
#
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
class HistoAlgo ( _GaudiHistoAlg , iAlgorithm ) :
    """
*******************************************************************************
*                                                * 'Physisics do not like it, *
*                                                *  physisics do not need it, *
*                                                *  physisics do not use  it' *
*                                                * ****************************
*  Usage:                                                                     *
*                                                                             *
*  from GaudiPython.GaudiAlgs import HistoAlgo, SUCCESS                       *
*                                                                             *
*  class MyClass(HistoAlgo) :                                                 *
*       ' My specific Algorithm, derived from GaudiAlgo base class '          *
*       def __init__( self , name , **args ) :                                *
*            'Constructor from algorithm instance name'                       *
*             #invoke the constructor of base class                           *
*             HistoAlgo.__init__(self , name , **args )                       *
*                                                                             *
*       def execute ( self ) :                                                *
*            'Major method (from IAlgorithm interface)'                       *
*                                                                             *
*           # get some data from Transient Event Store                        *
*           tracks = self.get('/Event/Rec/Tracks')                            *
*                                                                             *
*           self.plot1D ( tracks->size() , '#tracks' , 0 , 100 )              *
*                                                                             *
*           return SUCCESS                                                    *
*                                                                             *
* Alternatively the histogram  could be booked in advance:                    *
*                                                                             *
*  class MyClass(HistoAlgo) :                                                 *
*       ' My specific Algorithm, derived from GaudiAlgo base class '          *
*       def __init__( self , name ) :                                         *
*            'Constructor from algorithm instance name'                       *
*             #invoke the constructor of base class                           *
*             HistoAlgo.__init__(self , name )                                *
*                                                                             *
*       def initialize ( self ) :                                             *
*           'Algorithm initialization'                                        *
*           # initialize the base class                                       *
*           status = HistoAlgo.initialize( self )                             *
*           if status.isFailure() : return status                             *
*                                                                             *
*           # book the histogram                                              *
*           self.h1 = selff.book1D ( '#tracks' , 0 , 100 )                    *
*                                                                             *
*           return SUCCESS                                                    *
*                                                                             *
*                                                                             *
*       def execute ( self ) :                                                *
*            'Major method (from IAlgorithm interface)'                       *
*                                                                             *
*           # get some data from Transient Event Store                        *
*           tracks = self.get('/Event/Rec/Tracks')                            *
*                                                                             *
*           # fill the histogram                                              *
*           self.h1.fill ( tracks->size() )                                   *
*                                                                             *
*           return SUCCESS                                                    *
*                                                                             *
*******************************************************************************
    """
    pass

# =============================================================================
## @class TupleAlgo
#  The base class for easy manupulations with N-Tuples
#
#  Usage:
#
#  @code
#
#  from GaudiPython.GaudiAlgs import TupleAlgo, SUCCESS
#
#  class MyClass(TupleAlgo) :
#       ' My specific Algorithm, derived from TupleAlgo base class '
#       def __init__( self , name , **args ) :
#            'Constructor from algorithm instance name& parameters'
#             #invoke the constructor of base class
#             TupleAlgo.__init__(self , name , **args )
#
#       def execute ( self ) :
#            'Major method (from IAlgorithm interface)'
#
#           # get some data from Transient Event Store
#           tracks = self.get('/Event/Rec/Tracks')
#
#           tup = self.nTuple('My N-Tuple')
#
#           for track in tracks :
#
#                 pt   = track.pt   ()
#                 p    = track.p    ()
#                 chi2 = track.chi2 ()
#
#                 #fill N-tuple:
#                 tup.column ( 'pt'   ,  pt   )
#                 tup.column ( 'p'    ,  p    )
#                 tup.column ( 'chi2' ,  chi2 )
#                 #commit the row
#                 tup.write  ()
#
#           return SUCCESS
#
#  @endcode
#
#  @see GaudiTupleAlg
#
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
class TupleAlgo ( _GaudiTupleAlg  , iAlgorithm ) :
    """
*******************************************************************************
*                                                * 'Physisics do not like it, *
*                                                *  physisics do not need it, *
*                                                *  physisics do not use  it' *
*                                                * ****************************
*  Usage:                                                                     *
*                                                                             *
*  from GaudiPython.GaudiAlgs import TupleAlgo, SUCCESS                       *
*                                                                             *
*  class MyClass(TupleAlgo) :                                                 *
*       ' My specific Algorithm, derived from TupleAlgo base class '          *
*       def __init__( self , name , **args ) :                                *
*            'Constructor from algorithm instance name & parameters'          *
*             #invoke the constructor of base class                           *
*             TupleAlgo.__init__(self , name , **args )                       *
*                                                                             *
*       def execute ( self ) :                                                *
*            'Major method (from IAlgorithm interface)'                       *
*                                                                             *
*           # get some data from Transient Event Store                        *
*           tracks = self.get('/Event/Rec/Tracks')                            *
*                                                                             *
*           tup = self.nTuple('My N-Tuple')                                   *
*                                                                             *
*           for track in tracks :                                             *
*                                                                             *
*                 pt   = track.pt   ()                                        *
*                 p    = track.p    ()                                        *
*                 chi2 = track.chi2 ()                                        *
*                                                                             *
*                 #fill N-tuple:                                              *
*                 tup.column ( 'pt'   ,  pt   )                               *
*                 tup.column ( 'p'    ,  p    )                               *
*                 tup.column ( 'chi2' ,  chi2 )                               *
*                 #commit the row                                             *
*                 tup.write  ()                                               *
*                                                                             *
*           return SUCCESS                                                    *
*                                                                             *
*******************************************************************************
    """
    pass
class objectmethod(object) :
    def __init__(self, m) :
        self.method = m
    def __call__(self, *args) :
        print args
        return self.method(*args )

GaudiAlgo._Base = _GaudiAlgorithm
HistoAlgo._Base = _GaudiHistoAlg
TupleAlgo._Base = _GaudiTupleAlg

# initialize is 'unique' method :
GaudiAlgo.initialize = _initialize_
HistoAlgo.initialize = _initialize_histo_
TupleAlgo.initialize = _initialize_tuple_

def _start_ ( self ) :
    """
    The stub 'start' method needed by the internal implementation of PyAlg<>.
    """
    # return self._Base.start_(self)
    return SUCCESS

GaudiAlgo.start = _start_
HistoAlgo.start = _start_
TupleAlgo.start = _start_

def _execute_ ( self ) :
    """
    The fictive 'execute' method, which MUST be overwitten by user
    """
    raise RuntimeError, 'Execute method is not implemented for %s' % self.name()

GaudiAlgo.execute = _execute_
HistoAlgo.execute = _execute_
TupleAlgo.execute = _execute_

def _stop_ ( self ) :
    """
    The stub 'stop' method needed by the internal implementation of PyAlg<>.
    """
    # return self._Base.stop_(self)
    return SUCCESS

GaudiAlgo.stop = _stop_
HistoAlgo.stop = _stop_
TupleAlgo.stop = _stop_

# =============================================================================
def _plot1D_    ( s, *a ) :
    """
    The basic method to fill (book-on-demand) 1D-histogram

    The histogram will be created/booked dautomatically according to the
    specifications:

       - literal or numerical ID (optional)
       - title
       - low edge
       - high edge
       - number of bins (default is 100)

    The reference to the histogram is returned and could be used for later manipulations

    """
    return HistoDecorator.plot1D    (s,*a)
# =============================================================================
def _plot2D_    ( s, *a ) :
    """
    The basic method to fill (book-on-demand) 2D-histogram

    The histogram will be created/booked dautomatically according to the
    specifications:

       - literal or numerical ID (optional)
       - title
       - low X-edge
       - high X-edge
       - low Y-edge
       - high Y-edge
       - number of X-bins (default is 50)
       - number of Y-bins (default is 50)

    The reference to the histogram is returned and could be used for later manipulations

    """
    return HistoDecorator.plot2D    (s,*a)
# =============================================================================
def _plot3D_    ( s, *a ) :
    """
    The basic method to fill (book-on-demand) 3D-histogram

    The histogram will be created/booked dautomatically according to the
    specifications:

       - literal or numerical ID (optional)
       - title
       - low X-edge
       - high X-edge
       - low Y-edge
       - high Y-edge
       - low Z-edge
       - high Z-edge
       - number of X-bins (default is 10)
       - number of Y-bins (default is 10)
       - number of Y-bins (default is 10)

    The reference to the histogram is returned and could be used for later manipulations

    """
    return HistoDecorator.plot3D    (s,*a)
# =============================================================================
def _profile1D_ ( s, *a ) :
    """
    The basic method to fill (book-on-demand) 1D profile histogram

    The profile histogram will be created/booked dautomatically
    according to the specifications:

       - literal or numerical ID (optional)
       - title
       - low X-edge
       - high X-edge
       - number of X-bins (default is 100)

    The reference to the histogram is returned and could be used for later manipulations

    """
    return HistoDecorator.profile1D (s,*a)
# =============================================================================
def _profile2D_ ( s, *a ) :
    """
    The basic method to fill (book-on-demand) 2D profile histiogram

    The profile histogram will be created/booked automatically
    according to the specifications:

       - literal or numerical ID (optional)
       - title
       - low X-edge
       - high X-edge
       - low Y-edge
       - high Y-edge
       - number of X-bins (default is 50)
       - number of Y-bins (default is 50)

    The reference to the histogram is returned and could be used for later manipulations

    """
    return HistoDecorator.profile2D (s,*a)
# =============================================================================

_plot1D_    .__doc__ += '\n' + HistoDecorator.plot1D    .__doc__
_plot2D_    .__doc__ += '\n' + HistoDecorator.plot2D    .__doc__
_plot3D_    .__doc__ += '\n' + HistoDecorator.plot3D    .__doc__
_profile1D_ .__doc__ += '\n' + HistoDecorator.profile1D .__doc__
_profile2D_ .__doc__ += '\n' + HistoDecorator.profile2D .__doc__

def _decorate_plots_ ( klasses ) :
    t = type( klasses )
    if not issubclass ( t , list  ) and  \
       not issubclass ( t , tuple ) : klasses = [ klasses ]
    for klass in klasses :
        klass .plot       = _plot1D_
        klass .plot1D     = _plot1D_
        klass .plot2D     = _plot2D_
        klass .plot3D     = _plot3D_
        klass .profile1D  = _profile1D_
        klass .profile2D  = _profile2D_

_decorate_plots_ ( HistoAlgo )
_decorate_plots_ ( TupleAlgo )


# =============================================================================
def _nTuple_ ( s , *a )  :
    """
    Retrieve (book-on-demand) N-Tuple object
    """
    return TupleAlgDecorator.nTuple ( s , *a )
# =============================================================================
def _evtCol_ ( s , *a )  :
    """
    Retrieve (book-on-demand) N-Tuple object for Event Tag Collections
    """
    return TupleAlgDecorator.evtCol ( s , *a )

_nTuple_.__doc__  += '\n' + TupleAlgDecorator.nTuple.__doc__
_evtCol_.__doc__  += '\n' + TupleAlgDecorator.evtCol.__doc__

def _decorate_tuples_ ( klasses ) :
    t = type( klasses )
    if not issubclass ( t , list  ) and  \
       not issubclass ( t , tuple ) : klasses = [ klasses ]
    for klass in klasses :
        klass . nTuple    = _nTuple_
        klass . evtCol    = _evtCol_
        klass . ntupleSvc = _ntupleSvc
        klass .  tupleSvc = _ntupleSvc
        klass .   ntupSvc = _ntupleSvc
        klass .    tupSvc = _ntupleSvc
        klass . evtColSvc = _evtcolSvc
        klass . evtcolSvc = _evtcolSvc

# ==========================================================
_decorate_tuples_ ( TupleAlgo )


# "decorate N-Tuple object
Tuple = cpp.Tuples.Tuple
_Dec  = TupleDecorator
def _t_nTuple_      ( s , *a ) :
    """
    Access to underlying INTuple object
    """
    return _Dec.nTuple     ( s , *a )
def _t_ntuple_      ( s , *a ) :
    """
    Access to underlying NTuple::Tuple object
    """    
    return _Dec.ntuple     ( s , *a )
def _t_valid_       ( s , *a ) :
    """
    Valid NTuple::Tuple object?
    """
    return _Dec.valid      ( s , *a )
def _t_write_       ( s , *a ) :
    """
    Commit the row/record to n-tuple 
    """
    return _Dec.write      ( s , *a )
def _t_column_      ( s , *a ) :
    """
    Fill the certain column to n-tuple 
    """
    return _Dec.column     ( s , *a )
def _t_column_ll_   ( s , *a ) :
    """
    Fill the 'long long' column 
    """
    return _Dec.column_ll  ( s , *a )
def _t_column_ull_  ( s , *a ) :
    """
    Fill the 'unsigned long long' column 
    """
    return _Dec.column_ull ( s , *a )
def _t_array_       ( s , *a ) :
    """
    Fill the fixed-size array column 
    """
    return _Dec.array      ( s , *a )
def _t_matrix_      ( s , *a ) :
    """
    Fill the fixed-size matrix column 
    """
    return _Dec.matrix     ( s , *a )
def _t_farray_      ( s , *a ) :
    """
    Fill the floating-size array column 
    """
    return _Dec.farray     ( s , *a )
def _t_fmatrix_     ( s , *a ) :
    """
    Fill the floating-size matrix column 
    """
    return _Dec.fmatrix    ( s , *a )

_t_nTuple_     . __doc__ += '\n' + _Dec.nTuple     . __doc__
_t_ntuple_     . __doc__ += '\n' + _Dec.ntuple     . __doc__
_t_valid_      . __doc__ += '\n' + _Dec.valid      . __doc__
_t_write_      . __doc__ += '\n' + _Dec.write      . __doc__
_t_column_     . __doc__ += '\n' + _Dec.column     . __doc__
_t_column_ll_  . __doc__ += '\n' + _Dec.column_ll  . __doc__
_t_column_ull_ . __doc__ += '\n' + _Dec.column_ull . __doc__
_t_array_      . __doc__ += '\n' + _Dec.array      . __doc__
_t_matrix_     . __doc__ += '\n' + _Dec.matrix     . __doc__
_t_farray_     . __doc__ += '\n' + _Dec.farray     . __doc__
_t_fmatrix_    . __doc__ += '\n' + _Dec.fmatrix    . __doc__

Tuple.nTuple      = _t_nTuple_
Tuple.ntuple      = _t_ntuple_
Tuple.valid       = _t_valid_
Tuple.write       = _t_write_
Tuple.column      = _t_column_
Tuple.column_ll   = _t_column_ll_
Tuple.column_ull  = _t_column_ull_ 
Tuple.array       = _t_array_
Tuple.matrix      = _t_matrix_
Tuple.farray      = _t_farray_
Tuple.fmatrix     = _t_fmatrix_

_alg_map_ = {
    '__init__'    : _init_        , # constructor
    'tool'        : _tool_        , # service locator
    'svc'         : _service_     , # tool locator
    'evtSvc'      : _evtSvc       , # event data service
    'eventSvc'    : _evtSvc       , # event data service
    'detSvc'      : _detSvc       , # detector data service
    'histoSvc'    : _histoSvc     , # histogram data service
    'histSvc'     : _histoSvc     , # histogram data service
    'get'         : _get          , # access to  event data
    'get_'        : _get_         , # access to  event data
    'exist_'      : _exist_       , # check  the event data
    'getDet'      : _getDet       , # access to detector data
    'finalize'    : _finalize_    , # algorithm finalization
    'beginRun'    : _success_     , # dummy function returning success
    'endRun'      : _success_     , # dummy function returning success
    #
    'hasProperty' : _hasProperty_ , # check the existence of property with given name
    'getProperty' : _getProperty_ , # get the property value with given name
    'setProperty' : _setProperty_ , # set the property with given name
    '__setattr__' : _set_attr_    , # set the attribute/property with given name
    '__getattr__' : _get_attr_      # set the attribute/property with given name
    }


# decorate the classes with the useful methods
def _decorate_algs_ ( klasses ) :
    t = type( klasses )
    if not issubclass ( t , list  ) and  \
       not issubclass ( t , tuple ) : klasses = [ klasses ]
    for _alg in klasses :
        for key in _alg_map_ : setattr( _alg , key , _alg_map_[key] )

# =
_decorate_algs_ ( GaudiAlgo )
_decorate_algs_ ( HistoAlgo )
_decorate_algs_ ( TupleAlgo )

# =============================================================================
# Helper function to fill histogram/ntuple using 'map'-operation
# =============================================================================
def mapvct ( func , sequence , ovct = None ) :
    """ Helper function to fill histogram/ntuple using 'map'-operation """
    if not ovct :
        vct = GaudiPython.Vector
    else :
        vct = ovct
    if   hasattr( sequence, 'size' ) :
        vct.reserve ( vct.size() + sequence.size() )
    elif hasattr( sequence, '__len__' ) :
        vct.reserve ( vct.size() + len( sequence ) )
    for object in sequence :
        vct.push_back( func( object ) )
    if not ovct : return vct
# =============================================================================




# =============================================================================
# get the list of tools
# =============================================================================
def _get_all_tools_ ( self , method ) :
    """
    Get all tools
    """
    _tools = std.vector('IAlgTool*')()
    _func  = getattr ( AlgDecorator , method )
    _num   = _func ( self , _tools )
    if _tools.size() != _num :
        raise RuntimeError, 'Unable to extract Tools'
    _res = []
    for _tool in _tools : _res += [ iAlgTool ( _tool.name() , _tool ) ]
    return _res
# =============================================================================
def _Tools_a_ ( self ) :
    """
    Retrieve the list of tools,
        aquired by component through GaudiCommon<TYPE> base:

    >>> alg   = ...             ## get the algorithm
    >>> tools = alg.Tools()     ## get the tools
    >>> for tool in tools :
    ...        print tool

    """
    _cmp  = getattr ( self , '_ialg' )
    if not _cmp : self.retrieveInterface()
    _cmp  = getattr ( self , '_ialg' )
    return _get_all_tools_ ( _cmp , '_tools_a_' )
# =============================================================================
def _Tools_t_ ( self ) :
    """
    Retrieve the list of tools,
        aquired by component through GaudiCommon<TYPE> base:

    >>> tool  = ...              ## get the tool
    >>> tools = tool.Tools()     ## get the tools
    >>> for t in tools :
    ...        print t

    """
    _cmp  = getattr ( self , '_itool' )
    if not _cmp : self.retrieveInterface()
    _cmp  = getattr ( self , '_itool' )
    return _get_all_tools_ ( _cmp , '_tools_t_' )


# =============================================================================
# get the list of counters
# =============================================================================
## get all counters
def _get_all_counters_ ( self , method , name = None ) :
    """
    get all counters
    """
    _cnts = std.vector('const StatEntity*')()
    _nams = std.vector('std::string')      ()
    _func = getattr ( AlgDecorator , method )
    _num  = _func   ( self , _nams , _cnts )
    if _nams.size() != _num or _cnts.size() != _num  :
        raise RuntimeError, 'Unable to extract Counters'
    _res = {}
    for _i in range(0,_num) :
        _nam = _nams[_i]
        _cnt = _cnts[_i]
        _res [ _nam ] = _cnt
    if not name : return _res
    return _res.get( name , None )
# =============================================================================
## get all counters
def _Counters_a_ ( self , name = None ) :
    """
    Retrieve the counters, managed GaudiCommon<TYPE> base:

    >>> alg  = ...             ## get the algorithm
    >>> cnts = alg.Counters()  ## get the counters
    >>> for key in cnts :
    ...        print key, cnts[key]


    Retrieve the counter, managed GaudiCommon<TYPE> base by name:

    >>> alg = ...                        ## get the algorithm
    >>> cnt = alg.Counters('MyCounter')  ## get the counter
    >>> print cnt

    """
    _cmp  = getattr ( self , '_ialg' )
    if not _cmp : self.retrieveInterface()
    _cmp  = getattr ( self , '_ialg' )
    return _get_all_counters_ ( _cmp , '_counters_a_' , name )
# =============================================================================
def _Counters_t_ ( self , name = None ) :
    """
    Retrieve the counters, managed GaudiCommon<TYPE> base:

    >>> tool = ...              ## get the tool
    >>> cnts = tool.Counters()  ## get the counters
    >>> for key in cnts :
    ...        print key, cnts[key]


    Retrieve the counter, managed GaudiCommon<TYPE> base by name:

    >>> tool = ...                         ## get the tool
    >>> cnt  = tool.Counters('MyCounter')  ## get the counter
    >>> print cnt

    """
    _cmp  = getattr ( self , '_itool' )
    if not _cmp : self.retrieveInterface()
    _cmp  = getattr ( self , '_itool' )
    return _get_all_counters_ ( _cmp , '_counters_t_' , name )
# =============================================================================
# get the counter
# =============================================================================
def _get_counter_ ( self , method , name ) :
    """
    get the counter
    """
    _func = getattr ( AlgDecorator , method )
    return _func ( self , name )
# ==============================================================================
def _Counter_a_ ( self , name ) :
    """
    Retrieve the counter managed GaudiCommon<TYPE> base by name:

    >>> alg  = ...                     ## get the algorithm
    >>> cnt  = alg.Counter('#accept')  ## get the counter
    >>> print cnt

    """
    _cmp  = getattr ( self , '_ialg' )
    if not _cmp : self.retrieveInterface()
    _cmp  = getattr ( self , '_ialg' )
    return _get_counter_ ( _cmp , '_counter_a_' , name )
# ==============================================================================
def _Counter_t_ ( self , name ) :
    """
    Retrieve the counter managed GaudiCommon<TYPE> base by name:

    >>> tool = ...                      ## get the tool
    >>> cnt  = tool.Counter('#accept')  ## get the counter
    >>> print cnt

    """
    _cmp  = getattr ( self , '_itool' )
    if not _cmp : self.retrieveInterface()
    _cmp  = getattr ( self , '_itool' )
    return _get_counter_ ( _cmp , '_counter_t_' , name )

# =============================================================================
# get all histos
# =============================================================================
cpp.GaudiAlg.ID .__repr__ = cpp.GaudiAlg.ID.idAsString
cpp.GaudiAlg.ID . __str__ = cpp.GaudiAlg.ID.idAsString
cpp.StatEntity  .__repr__ = cpp.StatEntity.toString
cpp.StatEntity  . __str__ = cpp.StatEntity.toString
# =============================================================================
def _get_all_histos_  ( component , method , name ) :
    """
    Get All histogram form the component
    """
    _res = {}
    for _his in (  std.vector('AIDA::IProfile2D*'   ) ,
                   std.vector('AIDA::IProfile1D*'   ) ,
                   std.vector('AIDA::IHistogram3D*' ) ,
                   std.vector('AIDA::IHistogram2D*' ) ,
                   std.vector('AIDA::IHistogram1D*' ) ) :
        _his = _his()
        _ids = std.vector('GaudiAlg::ID')     ()
        _fun = getattr ( HistoDecorator , method )
        _num = _fun ( component , _ids , _his )
        if _ids.size() != _num or _his.size() != _num  :
            raise RuntimeError, 'Unable to extract Histos!'
        for _i in range(0,_num) :
            _id = _ids[ _i ]
            if   _id.numeric() : _id = _id.numericID  ()
            elif _id.literal() : _id = _id.literalID  ()
            else               : _id = _is.idAsString ()
            _res [ _id ] = _his[ _i ]

    if not name : return _res                          ## return the dictionary

    id = cpp.GaudiAlg.ID ( name ) 
    for i in ( name            ,
               id.literalID () ,
               id.numericID () ,
               id.idAsString() , id  ) :
        h = _res.get( i , None  )
        if not not h : return h   ## return the histogram
        
    return None
# =============================================================================
def _Histos_a_ ( self , name = None ) :
    """
    Retrieve all histograms & profiles, booked through GauydiHistos<TYPE> base:

    >>> alg = ...              ## get the algorithm
    >>> histos = alg.Histos()  ## get all histograms & profiles
    >>> for key in histos :
    ...        print key, histos[key]

    Retrive the histogram with the certain ID :

    >>> alg = ...                           ## get the algorithm
    >>> histo = alg.Histos('some histo ID') ## get the histo by ID
    >>> print histo

    """
    _cmp  = getattr ( self , '_ialg' )
    if not _cmp : self.retrieveInterface()
    _cmp  = getattr ( self , '_ialg' )
    return _get_all_histos_ ( _cmp , '_histos_a_' , name )
# =============================================================================
def _Histos_t_ ( self , name = None ) :
    """
    Retrieve all histograms & profiles, booked through GauydiHistos<TYPE> base:

    >>> tool = ...              ## get the tool
    >>> histos = tool.Histos()  ## get all histograms & profiles
    >>> for key in histos :
    ...        print key, histos[key]

    Retrive the historgam with certain ID :

    >>> tool = ...                           ## get the tool
    >>> histo = tool.Histos('some histo ID') ## get the histo by ID
    >>> print histo

    """
    _cmp  = getattr ( self , '_itool' )
    if not _cmp : self.retrieveInterface()
    _cmp  = getattr ( self , '_itool' )
    return _get_all_histos_ ( _cmp , '_histos_t_' , name )
# =============================================================================


_Tools_a_    . __doc__ += '\n' + AlgDecorator   . _tools_a_    . __doc__
_Tools_t_    . __doc__ += '\n' + AlgDecorator   . _tools_t_    . __doc__
_Counters_a_ . __doc__ += '\n' + AlgDecorator   . _counters_a_ . __doc__
_Counters_t_ . __doc__ += '\n' + AlgDecorator   . _counters_t_ . __doc__
_Counter_a_  . __doc__ += '\n' + AlgDecorator   . _counter_a_  . __doc__
_Counter_t_  . __doc__ += '\n' + AlgDecorator   . _counter_t_  . __doc__
_Histos_a_   . __doc__ += '\n' + HistoDecorator . _histos_a_   . __doc__
_Histos_t_   . __doc__ += '\n' + HistoDecorator . _histos_t_   . __doc__

iAlgorithm   . Tools    = _Tools_a_
iAlgTool     . Tools    = _Tools_t_
iAlgorithm   . Counters = _Counters_a_
iAlgTool     . Counters = _Counters_t_
iAlgorithm   . Counter  = _Counter_a_
iAlgTool     . Counter  = _Counter_t_
iAlgorithm   . Histos   = _Histos_a_
iAlgTool     . Histos   = _Histos_t_

## finally add some decoration for histograms
import GaudiPython.HistoUtils

# =============================================================================
# pseudo help
# =============================================================================
def _help_() :
    print __doc__ , __author__
    print '\t\t\tDoc-string for class GaudiAlgo \n' , GaudiAlgo.__doc__
    print '\t\t\tDoc-string for class HistoAlgo \n' , HistoAlgo.__doc__
    print '\t\t\tDoc-string for class TupleAlgo \n' , TupleAlgo.__doc__

# =============================================================================
# pseudo-test suite
# =============================================================================
if __name__ == '__main__' :
    _help_()

# =============================================================================
# The END
# =============================================================================

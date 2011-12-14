#!/usr/bin/env python
# =============================================================================
##  This module contains set of simple and useful utilities for booking and
#   manipulations with Gaudi-AIDA histograms, inspired by Thomas' request
#   @author Vanya BELYAEV ibelyaev@physics.syr.edu
#   @date 2007-08-03
# =============================================================================
"""
This module contains set of simple and useful utilities for booking and
manipulations with Gaudi-AIDA histograms, inspired by Thomas' request

The module contains following public symbols:

  - book       for booking of various 1D,2D&3D-histograms
  - bookProf   for booking of various 1D&2D-profiles
  - getAsAIDA  for retrieval of histograms/profiles from HTS in AIDA format
  - getAsROOT  for retrieval of histograms/profiles from HTS in ROOT format
  - fill       for smart filling of 1D-histogram (AIDA or ROOT)
  - aida2root  for conversion of AIDA histogram to ROOT
  - HistoStats for many statistical information
  - HistoFile  class for storing histograms to a file
  - histoDump  for dumping of the histogram in text format (a'la HBOOK)
  - dumpHisto  for dumping of the histogram in text format (a'la HBOOK)

"""
# =============================================================================
__author__ = "Vanya BELYAEV ibelyaev@physics.syr.edu"
# =============================================================================
__all__    = (
    'book'      , ##                book AIDA histogram using Histogram Service
    'bookProf'  , ##        book AIDA profile histogram using Histogram Service
    'getAsAIDA' , ## get the histogram form Histogram Service as AIDA histogram
    'getAsROOT' , ## get the histogram form Histogram Service as AIDA histogram
    'fill'      , ##              "power-fill" method for filling of histograms
    'aida2root' , ##                                     AIDA -> ROOT converter
    'HistoStats', ##                  statistical information for 1D-histograms
    'HistoFile' , ##                     class for storing histograms to a file
    'histoDump' , ##                     dump histogramintext format a'la HBOOK
    'dumpHisto'   ##                     dump histogramintext format a'la HBOOK
    )
# =============================================================================
## import core of Gaudi
import ROOT
from   GaudiPython.Bindings import AppMgr
from   GaudiPython.Bindings import iHistogramSvc
from   GaudiPython.Bindings import gbl as cpp
HID = cpp.GaudiAlg.ID

## global flag
useROOT = False

# =============================================================================
## Helper private auxiliary function to get Application Manager
def _getAppMgr   ( **kwargs  ) :
    """
    Helper private auxiliary function to get Application Manager
    """
    gaudi = kwargs.get ( 'gaudi' , None )
    if not gaudi : gaudi = AppMgr()
    if not gaudi : raise RuntimeError, 'Unable to get valid ApplicationMgr'

    state = gaudi._isvc.FSMState()
    if state < cpp.Gaudi.StateMachine.CONFIGURED  : gaudi.config     ()
    state = gaudi._isvc.FSMState()
    if state < cpp.Gaudi.StateMachine.INITIALIZED : gaudi.initialize ()

    return gaudi                                               ## RETURN

# =============================================================================
## Helper private auxiliary function to get iHistogramSvs
def _getHistoSvc ( **kwargs ) :
    """
    Helper private auxiliary function to get iHistogramSvs
    """
    svc = kwargs.get ( 'service' , None )
    if not svc : svc = kwargs.get ( 'svc' , None )
    else       : return svc                                    ## RETURN
    gaudi = _getAppMgr ( **kwargs )
    return gaudi.histsvc ()                                    ## RETURN

# =============================================================================
## Helper private auxiliary function to get iDataSvs
def _getEvtSvc ( **kwargs ) :
    """
    Helper private auxiliary function to get iDataSvs
    """
    svc = kwargs.get ( 'service' , None )
    if not svc : svc = kwargs.get ( 'svc' , None )
    else       : return svc                                    ## RETURN
    gaudi = _getAppMgr ( **kwargs )
    return gaudi.evtsvc()                                      ## RETURN

# =============================================================================
## The trivial function to book the various 1D,2D&3D-histograms
def book ( *args , **kwargs ) :
    """
    The trivial function to book the various 1D,2D&3D-histograms

    (1) book the trivial 1D histogram with full path

    >>> h1D = book ( 'path/to/my/histo'       , ## path in Histogram Transient Store
                     'cosine of decay angle ' , ## histogram title
                      100                     , ## number of bins
                      -1                      , ## low edge
                      100                     ) ## high edge

    (2) book the trivial 1D histogram with directory path and string ID :

    >>> h1D = book ( 'path/to/directory'      , ## the path to directory in HTS
                     'H1'                     , ## string histogram identifier
                     'cosine of decay angle ' , ## histogram title
                      100                     , ## number of bins
                      -1                      , ## low edge
                      100                     ) ## high edge

    (3) book the trivial 1D histogram with directory path and integer ID :

    >>> h1D = book ( 'path/to/directory'      , ## the path to directory in HTS
                     124                      , ## integer histogram identifier
                     'cosine of decay angle ' , ## histogram title
                      100                     , ## number of bins
                      -1                      , ## low edge
                      100                     ) ## high edge

    (4) book the trivial 2D histogram with full path

    >>> h1D = book ( 'path/to/my/histo'       , ## path in Histogram Transient Store
                     'm12**2 versus m13**2'   , ## histogram title
                      50                      , ## number of X-bins
                      1.0                     , ## low X-edge
                      4.0                     , ## high X-edge
                      50                      , ## number of Y-bins
                      1                       , ## low Y-edge
                      2                       ) ## high Y-edge

    (5) book the trivial 2D histogram with directory path and literal ID

    >>> h1D = book ( 'path/to/directory'      , ## path in Histogram Transient Store
                     'Dalitz1'                , ## literal histogram identifier
                     'm12**2 versus m13**2'   , ## histogram title
                      50                      , ## number of X-bins
                      1.0                     , ## low X-edge
                      4.0                     , ## high X-edge
                      50                      , ## number of Y-bins
                      1                       , ## low Y-edge
                      2                       ) ## high Y-edge

    (6) book the trivial 2D histogram with directory path and integer ID

    >>> h1D = book ( 'path/to/directory'      , ## path in Histogram Transient Store
                     854                      , ## integer histogram identifier
                     'm12**2 versus m13**2'   , ## histogram title
                      50                      , ## number of X-bins
                      1.0                     , ## low X-edge
                      4.0                     , ## high X-edge
                      50                      , ## number of Y-bins
                      1.0                     , ## low Y-edge
                      4.0                     ) ## high Y-edge

    (7) book the trivial 3D histogram with full path

    >>> h1D = book ( 'path/to/my/histo'       , ## path in Histogram Transient Store
                     'x vs y vs z '           , ## histogram title
                      10                      , ## number of X-bins
                      -1.0                    , ## low X-edge
                      1.0                     , ## high X-edge
                      10                      , ## number of Y-bins
                      -1.0                    , ## low Y-edge
                      1.0                     , ## high Y-edge
                      10                      , ## number of Z-bins
                      -1.0                    , ## low Z-edge
                      1.0                     ) ## high Z-edge

    (8) book the trivial 3D histogram with directory path and literal ID

    >>> h1D = book ( 'path/to/directory'      , ## path in Histogram Transient Store
                     'xyz'                    , ## literal histogram identifier
                     'x vs y vs z'            , ## histogram title
                      10                      , ## number of X-bins
                      -1.0                    , ## low X-edge
                      1.0                     , ## high X-edge
                      10                      , ## number of Y-bins
                      -1.0                    , ## low Y-edge
                      1.0                     , ## high Y-edge
                      10                      , ## number of Z-bins
                      -1.0                    , ## low Z-edge
                      1.0                     ) ## high Z-edge

    (9) book the trivial 3D histogram with directory path and integer ID

    >>> h1D = book ( 'path/to/directory'      , ## path in Histogram Transient Store
                     888                      , ## integer histogram identifier
                     'x vs y vs z'            , ## histogram title
                      10                      , ## number of X-bins
                      -1.0                    , ## low X-edge
                      1.0                     , ## high X-edge
                      10                      , ## number of Y-bins
                      -1.0                    , ## low Y-edge
                      1.0                     , ## high Y-edge
                      10                      , ## number of Z-bins
                      -1.0                    , ## low Z-edge
                      1.0                     ) ## high Z-edge

    Many other booking methods are available,
    e.g. for the histograms with non-equidistant bins, see IHistogamSvc::book

    """
    if useROOT or kwargs.get( 'useROOT' , False ) or not kwargs.get('useAIDA' , True ) :
        from ROOT import TH1D
        a0 = args[0]
        a1 = args[1]
        a2 = args[2]
        if not str is type(a1) :
            a1 = 'h'+str(a1)
        if     str is type(a2) :
            return TH1D ( a0+a1 , a2 , *args[3:] )
        else :
            return TH1D ( a0    , a1 , *args[2:] )

    svc = _getHistoSvc ( **kwargs )
    if not svc : raise RuntimeError, 'Unable to get valid HistogramService '
    ## book the histogram using the service
    return svc.book(*args)                                          ## RETURN

book.__doc__ += '\n\n' + '\thelp(iHistogramSvc.book) : \n\n' \
                + iHistogramSvc.book       . __doc__
book.__doc__ += '\n\n' + '\thelp(IHistogramSvc::book) : \n\n'            \
                + cpp.IHistogramSvc.book . __doc__

# =============================================================================
## The trivial function to book 1D&2D profile histograms:
def bookProf ( *args , **kwargs ) :
    """

    The trivial function to book 1D&2D profile histograms:

    (1) book 1D-profile histogram with full path in Histogram Transient Store:
    >>> histo = bookProf ( 'path/to/my/profile'  , ## path in Histogram Transient Store
                           'Energy Correction'   , ## the histogram title
                           100                   , ## number of X-bins
                           0.0                   , ## low X-edge
                           100                   ) ## high X-edge

    (2) book 1D-profile histogram with directory path and literal ID
    >>> histo = bookProf ( 'path/to/my/profile'  , ## path in Histogram Transient Store
                           'Calibration'         , ## the histogram literal identifier
                           'Energy Correction'   , ## the histogram title
                           100                   , ## number of X-bins
                           0.0                   , ## low X-edge
                           100                   ) ## high X-edge

    (3) book 1D-profile histogram with directory path and integer  ID
    >>> histo = bookProf ( 'path/to/my/profile'  , ## path in Histogram Transient Store
                           418                   , ## the histogram integer identifier
                           'Energy Correction'   , ## the histogram title
                           100                   , ## number of X-bins
                           0.0                   , ## low X-edge
                           100                   ) ## high X-edge

    (4) book 2D-profile histogram with full path in Histogram Transient Store:
    >>> histo = bookProf ( 'path/to/my/profile'  , ## path in Histogram Transient Store
                           'Energy Correction'   , ## the histogram title
                           50                    , ## number of X-bins
                           0.0                   , ## low X-edge
                           100                   , ## high X-edge
                           50                    , ## number of Y-bins
                           0.0                   , ## low Y-edge
                           100                   ) ## high Y-edge

    (5) book 2D-profile histogram with directory path and literal ID
    >>> histo = bookProf ( 'path/to/my/profile'  , ## path in Histogram Transient Store
                           'Calibration'         , ## the histogram literal identifier
                           'Energy Correction'   , ## the histogram title
                           50                    , ## number of X-bins
                           0.0                   , ## low X-edge
                           100                   , ## high X-edge
                           50                    , ## number of Y-bins
                           0.0                   , ## low Y-edge
                           100                   ) ## high Y-edge

    (6) book 2D-profile histogram with directory path and integer  ID
    >>> histo = bookProf ( 'path/to/my/profile'  , ## path in Histogram Transient Store
                           418                   , ## the histogram integer identifier
                           'Energy Correction'   , ## the histogram title
                           50                    , ## number of X-bins
                           0.0                   , ## low X-edge
                           100                   , ## high X-edge
                           50                    , ## number of Y-bins
                           0.0                   , ## low Y-edge
                           100                   ) ## high Y-edge

    Many other booking methods are available,
    e.g. for the histograms with non-equidistant bins, see IHistogamSvc::book

    """
    svc = _getHistoSvc ( **kwargs )
    if not svc : raise RuntimeError, 'Unable to get valid HistogramService '
    ## book the histogram using the service
    return svc.bookProf(*args)                                      ## RETURN


bookProf.__doc__ += '\n\n' + '\thelp(iHistogramSvc.bookProf) : \n\n' \
                    + iHistogramSvc.bookProf . __doc__
bookProf.__doc__ += '\n\n' + '\thelp(IHistogramSvc::bookProf) : \n\n'             \
                    + cpp.IHistogramSvc.bookProf . __doc__

# =============================================================================
## The most trivial function to retrieve the histogram from Histogram Transient Store
def getAsAIDA ( path , **kwargs ) :
    """

    The most trivial function to retrieve the histogram from Histogram Transient Store
    The histogram is returned by reference to its AIDA-representation (if possible)

    >>> h = getAsAIDA ( 'some/path/to/my/histogram' )

    """
    svc = _getHistoSvc ( **kwargs )
    if not svc : raise RuntimeError, 'Unable to get valid HistogramService '
    ## return the histogram
    return svc.getAsAIDA( path )                                   ## RETURN

getAsAIDA.__doc__ += '\n\n' + '\thelp(iHistogramSvc.getAsAIDA) : \n\n' \
                     + iHistogramSvc.getAsAIDA . __doc__
getAsAIDA.__doc__ += '\n\n' + '\thelp(iHistogramSvc.retrieve)  : \n\n' \
                     + iHistogramSvc.retrieve  . __doc__

# =============================================================================
## The most trivial function to retrieve the histogram from Histogram Transient Store
def getAsROOT ( path , **kwargs ) :
    """

    The most trivial function to retrieve the histogram from Histogram Transient Store
    The histogram is returned by reference to its underlying native ROOT-representation (if possible)

    >>> h = getAsROOT ( 'some/path/to/my/histogram' )

    """
    svc = _getHistoSvc ( **kwargs )
    if not svc : raise RuntimeError, 'Unable to get valid HistogramService '
    ## return the histogram
    return svc.getAsROOT( path )                                   ## RETURN

getAsROOT.__doc__ += '\n\n' + '\thelp(iHistogramSvc.getAsROOT) : \n\n' \
                     + iHistogramSvc.getAsROOT . __doc__


# =============================================================================
## The function which allows 'the smart fill' of 1D-histogram
def fill ( histo                   ,   ## histogram
           data                    ,   ## input data
           fun   = lambda x : x    ,   ## function to be used
           cut   = lambda x : True ,   ## cut to be applied
           **kwargs                ) : ## optional extra arguments
    """

    The function which allows 'the smart fill' of 1D-histogram

    >>> histo = ...

    The most trivial case, fill with the value
    >>> fill ( histo , 1.0 )

    Fill from any iterable object (sequence)
    >>> fill ( histo , [1,,2,3,4,5,10] )

    Fill from iterable object and apply the function:
    >>> fill ( histo , [1,2,3,4,5] , math.sin )

    Use lambda form:
    >>> fill ( histo , [1,2,3,4,5] , lambda x : x*x )

    The same
    >>> fill ( histo , [1,2,3,4,5] , fun = lambda x : x*x )

    Use internal attributes:
    >>> tracks = evtSvc['Rec/Track/Best']    ## iterable container of tracks
    >>> fill ( histo , tracks , lambda t : t.pt() )

    Apply the predicate: fill only even numbers:
    >>> fill ( histo , [1,2,3,4,5,6,7] , lambda x : x , lambda y : y%2 )

    The same (omit the trivial function) :
    >>> fill ( histo , [1,2,3,4,5,6,7] , cut = lambda y : y%2 )

    Apply the predicate: fill only pt for positively charged tracks:
    >>> tracks = evtSvc['Rec/Track/Best']
    >>> fill ( histo , tracks , lambda t : t.pt() , lambda t : 0<t.charge() )

    The same:
    >>> fill ( histo , tracks ,
               fun = lambda t : t.pt()       ,
               cut = lambda t : 0<t.charge() )

    Ordinary functions are also fine:
    >>> def myfun ( track ) : return sin( track.pt() + track.p() )
    >>> def mycut ( track ) : return track.p() > 100 * GeV
    >>> fill ( histo , tracks , myfun , mycut )

    The 'data' could be the address in TES, in this case the object
    is retrieved from TES and the method is applied to the objects,
    retrieved from TES:
    >>> fill (  histo             , ## the reference to the histogram
               'Rec/Track/Best'   , ## the location of objects in TES
                lambda t : t.pt() ) ## function to be used for histogram fill
    >>> fill (  histo             , ## the reference to the histogram
               'Rec/Track/Best'   , ## the address of objects in TES
                lambda t : t.pt() , ## the function to be used for histogram fill
                lambda t : t.charge()>0 ) ## the criteria to select tracks

    The arguments 'fun' and 'cut' could be strings, in this case
    they are evaluated by python before usage.
    This option could be often very useful.

    """

    # if value is a string, try to get the objects from TES
    if type ( data ) == str :
        svc  = _getEvtSvc ( **kwargs )
        data = svc[data]
        return fill ( histo , data , fun , cut , **kwargs )

    # if the function  is a string: evaluate it!
    if type ( fun  ) == str : fun  = eval ( fun  , globals() )

    # if the criterion is a string: evaluate it!
    if type ( cut  ) == str : cut  = eval ( cut  , globals() )

    if not hasattr ( data , '__iter__' ) : data = [ data ]

    if not hasattr ( histo , 'fill' ) and hasattr ( histo , 'Fill' ) :
        setattr ( histo , 'fill' , getattr ( histo , 'Fill' ) )

    for item in data :
        if not cut ( item )         : continue             ## CONTINUE
        histo.fill ( fun ( item ) )

    return histo                                           ## RETURN

# =============================================================================
## AIDA -> ROOT converter
aida2root = cpp.Gaudi.Utils.Aida2ROOT.aida2root
# =============================================================================
## Convert AIDA to ROOT
def _to_root_ ( self ) :
    """
    Convert AIDA to ROOT

    >>> aida = ...            ## get AIDA histogram
    >>> root = aida.toROOT()  ## convert it to ROOT

    """
    return aida2root ( self )

_to_root_ . __doc__  += aida2root . __doc__

for t in ( cpp.AIDA.IHistogram3D ,
           cpp.AIDA.IHistogram2D ,
           cpp.AIDA.IHistogram1D ,
           cpp.AIDA.IProfile2D   ,
           cpp.AIDA.IProfile1D   ) :
    if not hasattr ( t , 'Fill' ) and hasattr ( t , 'fill' ) :
        setattr ( t , 'Fill' , getattr ( t , 'fill' ) )
    for attr in ( 'toROOT' , 'toRoot' ,
                  'asROOT' , 'asRoot' ,
                  'AsROOT' , 'AsRoot' ) :
        if not hasattr ( t , attr ) : setattr ( t , attr , _to_root_ )

cpp.AIDA.IHistogram3D. __repr__ =  lambda s : cpp.GaudiAlg.Print3D.toString( s , HID( s.title() ) )
cpp.AIDA.IHistogram3D. __str__  = cpp.AIDA.IHistogram3D. __repr__


HistoStats = cpp.Gaudi.Utils.HistoStats

# =============================================================================
## Evaluate 'bin-by-bin' momentum of certain order around the value
def _moment_ ( self , order , value = 0 ) :
    """
    Evaluate 'bin-by-bin' momentum of order 'order' around the value 'value'
    for 1D histogram

    >>> h1 = ...
    >>> print h1.moment ( 5 )

    """
    return HistoStats.moment ( self , order , value )

# =============================================================================
## Evaluate error in 'bin-by-bin' momentum of certain order around the value
def _momentErr_ ( self , order ) :
    """
    Evaluate error for 'bin-by-bin' momentum of order 'order' around the value 'value'
    for 1D histogram

    >>> h1 = ...
    >>> print h1.momentErr ( 5 )

    """
    return HistoStats.momentErr ( self , order )
# =============================================================================
## Evaluate 'bin-by-bin' central momentum (around mean value)
def _centralMoment_ ( self , order ) :
    """
    Evaluate 'bin-by-bin' central momentum (around mean value)
    for 1D histogram

    >>> h1 = ...
    >>> print h1.centralMoment ( 5 )

    """
    return HistoStats.centralMoment ( self , order )

# =============================================================================
## Evaluate error in 'bin-by-bin' momentum of certain order around the value
def _centralMomentErr_ ( self , order ) :
    """
    Evaluate error for 'bin-by-bin' central momentum (around mean value)
    for 1D histogram

    >>> h1 = ...
    >>> print h1.centralMomentErr ( 5 )

    """
    return HistoStats.centralMomentErr ( self , order )

# =============================================================================
## Evaluate 'bin-by-bin' skewness for 1D histogram
def _skewness_ ( self ) :
    """
    Evaluate 'bin-by-bin' skewness for 1D AIDA histogram

    >>> h1 = ...
    >>> print h1.skewness()

    """
    return HistoStats.skewness ( self )

# =============================================================================
## Evaluate error for 'bin-by-bin' skewness for 1D histogram
def _skewnessErr_ ( self ) :
    """
    Evaluate error for 'bin-by-bin' skewness

    >>> h1 = ...
    >>> print h1.skewnessErr()

    """
    return HistoStats.skewnessErr ( self )

# =============================================================================
## Evaluate 'bin-by-bin' kurtosis for 1D histogram
def _kurtosis_ ( self ) :
    """
    Evaluate 'bin-by-bin' kurtosis

    >>> h1 = ...
    >>> print h1.kurtosis ()

    """
    return HistoStats.kurtosis ( self )

# =============================================================================
## Evaluate error for 'bin-by-bin' kurtosis for 1D histogram
def _kurtosisErr_ ( self ) :
    """
    Evaluate error for 'bin-by-bin' kurtotis for 1D AIDA histogram

    >>> h1 = ...
    >>> print h1.kurtotisErr()

    """
    return HistoStats.kurtosisErr ( self )

# =============================================================================
def _nEff_    ( self ) :
    """
    Number of equivalent entries
    """
    return HistoStats.nEff ( self )
# =============================================================================
def _mean_    ( self ) :
    """
    Evaluate the MEAN value
    """
    return HistoStats.mean ( self )
# =============================================================================
def _meanErr_ ( self ) :
    """
    Evaluate the error for MEAN estimate
    """
    return HistoStats.meanErr ( self )

# =============================================================================
def _rms_    ( self ) :
    """
    Evaluate the RMS for AIDA histogram
    """
    return HistoStats.rms ( self )
# =============================================================================
def _rmsErr_ ( self ) :
    """
    Evaluate the error for RMS estimate
    """
    return HistoStats.rmsErr ( self )

# =============================================================================
def _sumBinHeightErr_    ( self ) :
    """
    Get an error in the sum bin height ('in-range integral')
    """
    return HistoStats.sumBinHeightErr ( self )

# =============================================================================
def _sumAllBinHeightErr_ ( self ) :
    """ Get an error in the sum bin height ('in-range integral') """
    return HistoStats.sumAllBinHeightErr ( self )

# =============================================================================
def _overflowEntriesFrac_     ( self ) :
    """
    The fraction of overflow entries  (useful for shape comparison)
    """
    return HistoStats.overflowEntriesFrac     ( self )
# =============================================================================
def _overflowEntriesFracErr_  ( self ) :
    """
    The error for fraction of overflow entries  (useful for shape comparison)
    """
    return HistoStats.overflowEntriesFracErr  ( self )
# =============================================================================
def _underflowEntriesFrac_    ( self ) :
    """
    The fraction of underflow entries  (useful for shape comparison)
    """
    return HistoStats.underflowEntriesFrac    ( self )
# =============================================================================
def _underflowEntriesFracErr_ ( self ) :
    """
    The error for fraction of underflow entries  (useful for shape comparison)
    """
    return HistoStats.underflowEntriesFracErr ( self )

# =============================================================================
def _overflowIntegralFrac_     ( self ) :
    """
    The fraction of overflow integral  (useful for shape comparison)
    """
    return HistoStats.overflowIntegralFrac     ( self )
# =============================================================================
def _overflowIntegralFracErr_  ( self ) :
    """
    The error for fraction of overflow integral  (useful for shape comparison)
    """
    return HistoStats.overflowIntegralFracErr  ( self )
# =============================================================================
def _underflowIntegralFrac_    ( self ) :
    """
    The fraction of underflow integral  (useful for shape comparison)
    """
    return HistoStats.underflowIntegralFrac    ( self )
# =============================================================================
def _underflowIntegralFracErr_ ( self ) :
    """
    The error for fraction of underflow integral (useful for shape comparison)
    """
    return HistoStats.underflowIntegralFracErr ( self )

# =============================================================================
## get number of entries in histogram up to  the certain bin (not-included)
#  get number of entries in histogram form the certain
#  minimal bin up to the certain maximal bin (not-included)
def _nEntries_ ( self , i1 , i2 = -10000000 ) :
    """
    Get number of entries in histogram up to  the certain bin (not-included)

    attention: underflow bin is included!

    >>> h1
    >>> print h1.nEntries ( 10 )

    Get number of entries in histogram form the certain
    minimal bin up to the certain maximal bin (not-included)

    >>> h1
    >>> print h1.nEntries ( 10 , 15 )

    """
    if  i2 < i1 or i2 < 0 : return HistoStats.nEntries ( self , i1 )
    return HistoStats.nEntries ( self , i1 , i2 )
# =============================================================================
def _nEntriesFrac_ ( self , i1 , i2 = -10000000 ) :
    """
    Get the fraction of entries in histogram up to  the certain bin (not-included)

    attention: underflow bin is included!

    >>> h1
    >>> print h1.nEntriesFrac ( 10 )

    Get the fraction of entries in histogram form the certain
    minimal bin up to the certain maximal bin (not-included)

    >>> h1
    >>> print h1.nEntriesFrac ( 10 , 15 )

    """
    if  i2 < i1 or i2 < 0 : return HistoStats.nEntriesFrac ( self , i1 )
    return HistoStats.nEntriesFrac ( self , i1 , i2 )
# =============================================================================
def _nEntriesFracErr_ ( self , i1 , i2 = -10000000 ) :
    """
    Get error for  fraction of entries in histogram up to  the certain bin (not-included)

    attention: underflow bin is included!

    >>> h1
    >>> print h1.nEntriesFracErr( 10 )

    Get error  fraction of entries in histogram form the certain
    minimal bin up to the certain maximal bin (not-included)

    >>> h1
    >>> print h1.nEntriesFracErr ( 10 , 15 )

    """
    if  i2 < i1 or i2 < 0 : return HistoStats.nEntriesFracErr ( self , i1 )
    return HistoStats.nEntriesFracErr ( self , i1 , i2 )

# =============================================================================
i1DH = cpp.AIDA.IHistogram1D

if not hasattr ( i1DH , 'moment'          ) : i1DH.moment           = _moment_
if not hasattr ( i1DH , 'momentErr'       ) : i1DH.momentErr        = _momentErr_
if not hasattr ( i1DH , 'centralMoment'   ) : i1DH.centralMoment    = _centralMoment_
if not hasattr ( i1DH , 'momentMomentErr' ) : i1DH.centralMomentErr = _centralMomentErr_
if not hasattr ( i1DH , 'nEff'            ) : i1DH.nEff             = _nEff_
if not hasattr ( i1DH , 'mean'            ) : i1DH.mean             = _mean_
if not hasattr ( i1DH , 'meanErr'         ) : i1DH.meanErr          = _meanErr_
if not hasattr ( i1DH , 'rms'             ) : i1DH.rms              = _rms_
if not hasattr ( i1DH , 'rmsErr'          ) : i1DH.rmsErr           = _rmsErr_
if not hasattr ( i1DH , 'skewness'        ) : i1DH.skewness         = _skewness_
if not hasattr ( i1DH , 'skewnessErr'     ) : i1DH.skewnessErr      = _skewnessErr_
if not hasattr ( i1DH , 'kurtosis'        ) : i1DH.kurtosis         = _kurtosis_
if not hasattr ( i1DH , 'kurtosisErr'     ) : i1DH.kurtosisErr      = _kurtosisErr_

if not hasattr ( i1DH , 'overflowEntriesFrac'     ) : i1DH.overflowEntriesFrac     = _overflowEntriesFrac_
if not hasattr ( i1DH , 'overflowEntriesFracErr'  ) : i1DH.overflowEntriesFracErr  = _overflowEntriesFracErr_
if not hasattr ( i1DH , 'underflowEntriesFrac'    ) : i1DH.underflowEntriesFrac    = _underflowEntriesFrac_
if not hasattr ( i1DH , 'underflowEntriesFracErr' ) : i1DH.underflowEntriesFracErr = _underflowEntriesFracErr_

if not hasattr ( i1DH , 'overflowIntegralFrac'     ) : i1DH.overflowIntegralFrac     = _overflowIntegralFrac_
if not hasattr ( i1DH , 'overflowIntegralFracErr'  ) : i1DH.overflowIntegralFracErr  = _overflowIntegralFracErr_
if not hasattr ( i1DH , 'underflowIntegralFrac'    ) : i1DH.underflowIntegralFrac    = _underflowIntegralFrac_
if not hasattr ( i1DH , 'underflowIntegralFracErr' ) : i1DH.underflowIntegralFracErr = _underflowIntegralFracErr_

if not hasattr ( i1DH , 'nEntries'        ) : i1DH.nEntries        = _nEntries_
if not hasattr ( i1DH , 'nEntriesFrac'    ) : i1DH.nEntriesFrac    = _nEntriesFrac_
if not hasattr ( i1DH , 'nEntriesFracErr' ) : i1DH.nEntriesFracErr = _nEntriesFracErr_

## ============================================================================
def _path_ ( self ) :
    """
    Get the path in THS for the given AIDA object:

    >>> aida =
    >>> print aida.path()

    """
    return cpp.Gaudi.Utils.Histos.path ( self )

iBH = cpp.AIDA.IBaseHistogram
if not hasattr ( iBH , 'path'     ) : iBH.path     = _path_
if not hasattr ( iBH , 'TESpath'  ) : iBH.TESpath  = _path_
if not hasattr ( iBH , 'location' ) : iBH.location = _path_


## =============================================================================
def __dumpHisto__ ( histo , *args ) :
    """

    Dump the histogram/profile in text format (a'la HBOOK)

    >>> histo
    >>> print dumpHisto ( histo )

    >>> print histo.dump()
    >>> print histo.dump( 20 , 20 )
    >>> print histo.dump( 20 , 20 , True )

    Uses:

    """
    return cpp.Gaudi.Utils.Histos.histoDump ( histo , *args )

__dumpHisto__ .__doc__ = '\n'  + cpp.Gaudi.Utils.Histos.histoDump . __doc__

# =============================================================================
## the actual function for text dump of the histogram
histoDump = __dumpHisto__
dumpHisto = __dumpHisto__

for t in  ( cpp.AIDA.IHistogram1D ,
            cpp.AIDA.IProfile1D   ,
            ROOT.TH1D             ,
            ROOT.TH1F             ,
            ROOT.TH1              ,
            ROOT.TProfile         ) :
    for method in ( 'dump'       ,
                    'dumpHisto'  ,
                    'dumpAsText' ) :
        if not hasattr ( t , method ) : setattr ( t , method , __dumpHisto__ )

# ==============================================================================
class HistoFile :
    """
    Class to write histograms to a ROOT file.
    hFile = HistoFile("myFile.root")
    myHisto = ...
    hFile.save(myHisto)
    myHisto0 = ...
    myHisto1 = ...
    myHisto2 = ...
    hFile.save(myHisto0, myHisto1, myHisto2)
    histoList = [h0, h1, h2, h3]
    hFile.save(histoList)
    ...
    hWriter.close()
    """
    __author__ = "Juan Palacios juan.palacios@nikhef.nl"

    def __init__(self, fileName) :
        self.file = ROOT.TFile(fileName, "RECREATE")
        from GaudiPython import gbl
        self.aida2root = gbl.Gaudi.Utils.Aida2ROOT.aida2root
        self.aidaTypes = [ gbl.AIDA.IHistogram1D,
                           gbl.AIDA.IHistogram2D,
                           gbl.AIDA.IHistogram3D,
                           gbl.AIDA.IProfile1D,
                           gbl.AIDA.IProfile2D,
                           gbl.AIDA.IHistogram    ]

    def __convertibleType(self, histo) :
        histoType = type(histo)
        for t in self.aidaTypes :
            if histoType == t : return True
        return False

    def save(self, *args) :
        """
        This function stores histograms on the file for future saving.
        It takes an arbitrary number of AIDA or ROOT histograms or
        a list of them.
        """
        if args :
            if type(args[0])==list :
                histoList = args[0]
            else :
                histoList = args
            for h in histoList :
                if self.__convertibleType(h) : h = self.aida2root(h)
                h.Write()

    def close(self) :
        self.file.Write()
        self.file.Close()

# =============================================================================
if '__main__' == __name__ :
    import sys
    print __doc__
    for o in __all__ :
        print o
        print sys.modules[__name__].__dict__[o].__doc__


# =============================================================================
# The END
# =============================================================================

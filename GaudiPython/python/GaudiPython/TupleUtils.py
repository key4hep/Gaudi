#!/usr/bin/env python
# =============================================================================
## This module contains set of simple and useful utilities to booking and
#  manipulation with N-Tuples (in the spirit of GaudiTuples<TYPE>)
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2007-08-04
# =============================================================================
"""
This module contains set of simple and useful utilities to booking and
manipulation with N-Tuples (in the spirit of GaudiTuples<TYPE>)

"""
# =============================================================================
__author__ = "Vanya BELYAEV ibelyaev@physics.syr.edu"
# =============================================================================
__all__ = (
    'nTuple'        ,  ## function to book/retrieve N-tuple
    'getNTuple'     ,  ## ditto
    'getNtuple'     ,  ## ditto
    'getntuple'     ,  ## ditto
    'gettuple'      ,  ## ditto
    'activeTuples'  ,  ## get the list of all active n-tuples
    'releaseTuples'    ## release all actibe N-tuples
    )
# =============================================================================

from   GaudiPython.Bindings  import gbl as cpp
from   GaudiPython.Bindings  import AppMgr
import GaudiPython.GaudiAlgs

_Tool = cpp.ITupleTool
_Deco = cpp.GaudiPython.TupleToolDecorator

# the list of aquired tools (to be released)
_TOOLS_ = []

# =============================================================================
## Helper private auxillary utility to get Tool Service
def _getToolSvc( **kwargs ) :
    """ Helper private auxillary utility to get Tool Service """
    svc = kwargs.get ( 'toolSvc' , None )
    if not svc : svc = kwargs.get ( 'toolsvc' , None )
    if not svc : svc = kwargs.get ( 'service' , None )
    if not svc : svc = kwargs.get ( 'svc'     , None )
    else       : return svc                                ## RETURN
    gaudi = kwargs.get ( 'gaudi' , None )
    if not gaudi : gaudi = AppMgr()
    return gaudi.toolsvc()                                 ## RETURN


# =============================================================================
## Retrive N-Tuple ( book on demand )
def _nTuple_ ( s , *args ) :
    """ Retrive N-tuple ( book on demand )  """
    print 'ARGS:' , args
    return _Deco.nTuple ( s , *args)

# =============================================================================
_nTuple_. __doc__ += "\n" + _Deco.nTuple . __doc__
_Tool.nTuple = _nTuple_
_Tool.ntuple = _nTuple_


# =============================================================================
## Retrieve (book-on-demand) 'Smart'-N-tuple object.
def nTuple ( dirpath , ID , ID2 = None , topdir = None , LUN = 'FILE1' ) :
    """
    Retrieve 'Smart'-N-tuple object.
    N-tuple is booked on-demand.

    Atetntion !!
    The logical unit LUN must be configured by N-Tuple Service

    Retrieve (book-n-demand) N-Tuple using
    the  directory name and the title:
    >>> t = nTuple ( 'the/path/to/directory' , ## the path to the directory
                     'N-tuple title'         , ## the title for N-Tuple
                      LUN = 'FILE1'          ) ## logical file unit

    Retrieve (book-n-demand) N-Tuple using
    the  directory name, literal ID and the title:
    >>> t = nTuple ( 'the/path/to/directory' , ## the path to the directory
                     'Tuple1'                , ## the literal ID for N-Tuple
                     'N-tuple title'         , ## the title for N-Tuple
                      LUN = 'FILE1'          ) ## logical file unit

    Retrieve (book-n-demand) N-Tuple using
    the  directory name, numerical ID and the title:
    >>> t = nTuple ( 'the/path/to/directory' , ## the path to the directory
                     124                     , ## the numerical ID for N-Tuple
                     'N-tuple title'         , ## the title for N-Tuple
                      LUN = 'FILE1'          ) ## logical file unit


    """
    toolSvc = _getToolSvc ()

    # construct the name of the intermediate TupleTool
    name = 'Tuple'+LUN+"/"
    if topdir : name += topdir
    name += dirpath
    name += "_%s"%ID
    if  ID2 : name += "_%s"%ID2
    name=name.replace ( '.'  , '_' )
    name=name.replace ( '/'  , '_' )
    name=name.replace ( '\\' , '_' )
    name=name.replace ( ' '  , '_' )

    ## define tool properties
    t0 = GaudiPython.iAlgTool( 'ToolSvc.'+name )
    t0.OutputLevel      = 1
    t0.NTupleLUN        = LUN
    t0.NTupleDir        = dirpath
    t0.PropertiesPrint  = False
    t0.OutputLevel      = 4
    if topdir : t0.NTupleTopDir = topdir

    ## get the tool from Tool service
    tool = toolSvc.create ( 'TupleTool'       ,
                            name              ,
                            interface = _Tool )

    ## check the properties and redefine them if needed
    t1 = GaudiPython.iAlgTool ( tool.name() , tool )
    if t1.NTupleLUN != LUN     : t1.NTupleLUN = LUN
    if t1.NTupleDir != dirpath : t1.NTupleDir = dirpath
    if topdir and ( t1.NTupleTopDir != topdir ) :
        t1.NTupleTopDir = topdir

    _TOOLS_.append ( tool )
    if not ID2 : return tool.nTuple ( ID )               ## RETURN

    return tool.nTuple ( ID , ID2 )                      ## RETURN


nTuple . __doc__ += "\n\t help(ITupleTool.nTuple) : \n" \
                    + _Tool.nTuple.__doc__

ntuple    = nTuple
getNTuple = nTuple
getNtuple = nTuple
getntuple = nTuple
getTuple  = nTuple
gettuple  = nTuple

# =============================================================================
## Return the list of active tools
def activeTuples () :
    """
    Return the list of active tools
    """
    return _TOOLS_

# =============================================================================
## Release the active tool/tuples
def releaseTuples () :
    """
    Release the active tool/tuples
    The method needs to be invoked explicitely at the end of the job
    """
    if not _TOOLS_ : return
    print ' %s/%s: release all pending ITupleTools: %s' % ( __file__     ,
                                                           __name__     ,
                                                           len(_TOOLS_) )
    from GaudiPython.Bindings import _gaudi
    if not _gaudi : return
    
    toolSvc = _getToolSvc()
    if toolSvc.isValid() : 
        while _TOOLS_ :
            t = _TOOLS_.pop()
            if not t : continue
            while 1 < t.refCount() :
                toolSvc._its.releaseTool( t )

# =============================================================================
def _TupleUtils_AtExit_ () :
    """
    AtExit function for GaudiPython.TupleUtils module
    """
    if activeTuples() :
        print 'WARNING: the list of local TupleTools is not empty!'
        print 'WARNING: please use GaudiPython.TupleUtils.releaseTuples() at the end'

import atexit
atexit.register ( _TupleUtils_AtExit_ )

# =============================================================================
if "__main__" == __name__ :
    import sys
    print __doc__ , __all__
    for o in __all__ :
        print "\n\n\t",o,"\n"
        print sys.modules[__name__].__dict__[o].__doc__

# =============================================================================
# The end
# =============================================================================

#!/usr/bin/env python
# =============================================================================
# $Id: Properties.py,v 1.4 2007/05/24 14:36:37 hmd Exp $
# =============================================================================
# CVS tag $Name:  $, version $Revision: 1.4 $
# =============================================================================
"""
*******************************************************************************
*                                                                             *
* Simple example (identical to C++ Properties.opts) which illustrated         *
* the basic job-properties and theinr C++/Python intercommunication           *
*                                                                             *
*******************************************************************************
"""
# =============================================================================
__author__ = 'Vanya BELYAEV ibelyaev@physics.syr.edu'
# =============================================================================
## @file
#  Simple example (identical to C++ Properties.opts) which illustrated
#  the basic job-properties and theinr C++/Python intercommunication
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2007-02-13
# =============================================================================

import gaudimodule

SUCCESS = gaudimodule.SUCCESS

# =============================================================================
## the configurtaion of the job
def configure ( gaudi = None ) :
    """ the configurtaion of the job """

    ## create applictaion manager if not done yet
    if not gaudi : gaudi = gaudimodule.AppMgr()

    ## read main configuration files
    gaudi.config ( files = [ '../options/Common.opts' ] )

    ## private algorithm configuration options

    gaudi.TopAlg = [ 'PropertyAlg' ]

    # test for the multiple inclusion of the same alg
    gaudi.TopAlg  += [ "PropertyAlg", "PropertyProxy" ]

    # test for the removal of an algorithm
    gaudi.TopAlg.remove("PropertyAlg")

    # Set output level threshold
    #    (2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )
    msgSvc = gaudi.service('MessageSvc')
    msgSvc.OutputLevel = 3

    ## event related parameters
    gaudi.EvtSel = 'NONE'
    gaudi.HistogramPersistency = 'NONE'

    # Algorithms Private Options

    alg = gaudi.algorithm('PropertyAlg')

    alg.OutputLevel = 3;

    alg.Int    = 101
    alg.Double = 101.1e+10
    alg.String = "hundred one"
    alg.Bool   = False

    alg.IntArray    = [ 1, 2, 3, 5 ]
    alg.DoubleArray = [ -11.0 , 2., 3.3, 0.4e-03 ]
    alg.StringArray = [ "one" , "two" , "four" ]
    alg.BoolArray   = [ False , True , False ]
    alg.EmptyArray  = []

    alg.PInt    = 101
    alg.PDouble = 101.E5
    alg.PString = "hundred one"
    alg.PBool   = True


    alg.PIntArray    = [ 1, 2, 3, 5 ]
    alg.PDoubleArray = [ 1.1 , 2., 3.3 ]
    alg.PStringArray = [ "one", "two", "four" ]
    alg.PBoolArray   = [  True , False , True , False ]

    proxy = gaudi.algorithm( "PropertyProxy" )
    proxy.String = "This is set by the proxy"

    msgSvc.setDebug   = [ "EventLoopMgr" ]
    msgSvc.setVerbose = [ "MsgTest"      ]

    return SUCCESS
# =============================================================================


# =============================================================================
# The actual job excution
# =============================================================================
if '__main__' == __name__ :

    print __doc__ , __author__

    gaudi = gaudimodule.AppMgr()
    configure( gaudi )
    gaudi.run(1)

    alg = gaudi.algorithm( 'PropertyAlg' )

    props = alg.properties()
    print 'Properties of %s ' % alg.name()
    for p in props :
        v = props[p].value()
        print "Python: Name/Value:  '%s' / '%s' "%(p,v)

# =============================================================================
# The END
# =============================================================================

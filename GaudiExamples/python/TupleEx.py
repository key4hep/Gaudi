#!/usr/bin/env python
# =============================================================================
# $Id: TupleEx.py,v 1.9 2007/10/29 14:29:47 mato Exp $
# =============================================================================
# CVS tag $Name:  $ , version $Revision: 1.9 $
# =============================================================================
## @file
#
#  Simple example which illustrate the usage of useful
#  algorithm  base class for N-Tuple manipulations
#
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
# =============================================================================
"""
*******************************************************************************
*                                                                             *
* Simple example which illustrate the usage of useful                         *
* algorithm  base class for N-Tuple manipulations                             *
*                                                                             *
*******************************************************************************
"""
# =============================================================================
__author__ = 'Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr'
# =============================================================================

import GaudiPython, math

Rndm    = GaudiPython.gbl.Rndm
SUCCESS = GaudiPython.SUCCESS

from   GaudiPython.GaudiAlgs   import TupleAlgo

# =============================================================================
## @class TupleEx
#  Simple algorithm which books&fills three histograms
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
class TupleEx(TupleAlgo) :
    """
    Simple algorithm which implicitely book&fill N-Tuples
    """
    ## standard constructor
    def __init__ ( self , name = 'TupleEx' ) :
        """ Constructor """
        TupleAlgo.__init__( self , name )

    ## the main execution method
    def execute( self ) :
        """
        The major method 'execute', it is invoked for each event
        """

        gauss = Rndm.Numbers( self.randSvc() , Rndm.Gauss( 0.0 , 1.0 ) )

        tup = self.nTuple('My trivial N-tuple')
        for i in range(0,100) :
            tup.column( 'a' , math.sin(i) )
            tup.column( 'b' , math.cos(i) )
            tup.column( 'c' , math.tan(i) )
            tup.column( 'g' , gauss()     )
            tup.write()

        return SUCCESS


# =============================================================================
## job configuration
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
def configure( gaudi = None  ) :
    """ Configuration of the job """

    if not gaudi : gaudi = GaudiPython.AppMgr()

    gaudi.JobOptionsType       = 'NONE'
    gaudi.EvtSel               = 'NONE'
    gaudi.HistogramPersistency = 'ROOT'

    gaudi.ExtSvc += ["NTupleSvc" ]

    ntSvc = gaudi.service('NTupleSvc')
    ntSvc.Output = [ "MYLUN DATAFILE='TupleEx.root' OPT='NEW' TYP='ROOT'" ]

    gaudi.config()

    gaudi.DLLs = [ 'GaudiAlg', 'RootHistCnv', ]

    alg = TupleEx()
    gaudi.setAlgorithms( [alg] )

    ## configure the property
    alg.NTupleLUN = 'MYLUN'

    return SUCCESS


# =============================================================================
## The actual job excution
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
if '__main__' == __name__ :
    print __doc__
    gaudi = GaudiPython.AppMgr()
    configure( gaudi )
    gaudi.run(20)

# =============================================================================
# The END
# =============================================================================

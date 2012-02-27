#!/usr/bin/env python
# =============================================================================
# $Id: TupleEx2.py,v 1.8 2008/09/30 13:07:04 marcocle Exp $
# =============================================================================
# CVS tag $Name:  $, version $Revision: 1.8 $
# =============================================================================
## @file
#
# Simple example which illustrate the usage of useful
# algorithm  base class for N-Tuple manipulations
#
# @author Vanya BELYAEV ibelyaev@physics.syr.edu
# @date 2006-11-26
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

Rndm        = GaudiPython.gbl.Rndm
Math        = GaudiPython.gbl.ROOT.Math
SUCCESS     = GaudiPython.SUCCESS

from   GaudiPython.GaudiAlgs   import TupleAlgo

# =============================================================================
## @class TupleEx2
#  Simple algorithm for advanced N-Tuple columns
#
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
class TupleEx2(TupleAlgo) :
    """
    Simple algorithm for advanced N-Tuple columns
    """

    ## standard constructor
    def __init__ ( self , name = 'TupleEx2' ) :
        """ Constructor """
        TupleAlgo.__init__( self , name )

    ## the main execution method
    def execute( self ) :
        """ The major method 'execute', it is invoked for each event """

        gauss = Rndm.Numbers ( self.randSvc() , Rndm.Gauss ( 0.0 , 1.0 ) )
        flat  = Rndm.Numbers ( self.randSvc() , Rndm.Flat  ( -10 , 10  ) )
        breit = Rndm.Numbers ( self.randSvc() , Rndm.BreitWigner  ( 0.0 , 1.0  ) )

        ## N-tuple with 4D-vectors
        tup = self.nTuple('Vectors-4D', 'N-tuple with 4D-vectors')
        for i in range(0,100) :

            lv1 = Math.PxPyPzEVector()

            lv1.SetPx ( gauss () )
            lv1.SetPy ( gauss () )
            lv1.SetPz ( gauss () )
            lv1.SetE  ( gauss () )

            tup.column( 'lv1' , lv1 )

            tup.write()

        ## N-tuple with 3D-vectors
        tup = self.nTuple('Vectors-3D', 'N-tuple with 3D-vectors')
        for i in range(0,100) :

            v1 = Math.XYZVector()
            v1.SetX ( gauss () )
            v1.SetY ( gauss () )
            v1.SetZ ( gauss () )

            tup.column ( "v1" , v1 )

            tup.write()

        ## N-tuple with 3D-points
        tup = self.nTuple('Points-3D', 'N-tuple with 3D-points')
        for i in range(0,100) :

            p1 = Math.XYZPoint()
            p1.SetX ( gauss () )
            p1.SetY ( gauss () )
            p1.SetZ ( gauss () )

            tup.column ( "p1" , p1 )

            tup.write()

        return SUCCESS

# =============================================================================
## job configuration
#
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
def configure( gaudi = None  ) :
    """
    Configuration of the job
    """

    if not gaudi : gaudi = GaudiPython.AppMgr()

    gaudi.JobOptionsType       = 'NONE'
    gaudi.EvtSel               = 'NONE'
    gaudi.HistogramPersistency = 'ROOT'

    gaudi.ExtSvc += ["NTupleSvc" ]

    ntSvc = gaudi.service('NTupleSvc')
    ntSvc.Output = [ "MYLUN DATAFILE='TupleEx2.root' OPT='NEW' TYP='ROOT'" ]

    gaudi.config()

    gaudi.DLLs = [ 'GaudiAlg', 'RootHistCnv', ]

    alg = TupleEx2()
    gaudi.setAlgorithms( [alg] )

    # configure the properties
    alg.NTupleLUN = 'MYLUN'

    return SUCCESS


# =============================================================================
## The actual job execution
#
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

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

            lv2 = Math.PtEtaPhiEVector()
            x = flat()
            y = flat()
            z = flat()
            e = flat()
            lv2.SetPxPyPzE(x, y, z, e)

            tup.column( 'lv1' , lv1 )
            tup.column( 'lv2' , lv2 )

            tup.write()

        ## N-tuple with 3D-vectors
        tup = self.nTuple('Vectors-3D', 'N-tuple with 3D-vectors')
        for i in range(0,100) :

            v1 = Math.XYZVector()
            v1.SetX ( gauss () )
            v1.SetY ( gauss () )
            v1.SetZ ( gauss () )

            v2 = Math.Polar3DVector()
            x = flat()
            y = flat()
            z = flat()
            v2.SetXYZ(x, y, z)

            v3 = Math.RhoEtaPhiVector()
            x = breit()
            y = breit()
            z = breit()
            v3.SetXYZ(x, y, z)

            v4 = Math.RhoZPhiVector()
            x = gauss()
            y = flat()
            z = breit()
            v4.SetXYZ(x, y, z)

            tup.column ( "v1" , v1 )
            tup.column ( "v2" , v2 )
            tup.column ( "v3" , v3 )
            tup.column ( "v4" , v4 )

            tup.write()

        ## N-tuple with 3D-points
        tup = self.nTuple('Points-3D', 'N-tuple with 3D-points')
        for i in range(0,100) :

            p1 = Math.XYZPoint()
            p1.SetX ( gauss () )
            p1.SetY ( gauss () )
            p1.SetZ ( gauss () )

            p2 = Math.Polar3DPoint()
            x = flat()
            y = flat()
            z = flat()
            p2.SetXYZ(x, y, z)

            p3 = Math.RhoEtaPhiPoint()
            x = breit()
            y = breit()
            z = breit()
            p3.SetXYZ(x, y, z)

            p4 = Math.RhoZPhiPoint()
            x = gauss()
            y = flat()
            z = breit()
            p4.SetXYZ(x, y, z)

            tup.column ( "p1" , p1 )
            tup.column ( "p2" , p2 )
            tup.column ( "p3" , p3 )
            tup.column ( "p4" , p4 )

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

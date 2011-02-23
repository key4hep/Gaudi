#!/usr/bin/env python2.4
# =============================================================================
# $Id: TupleEx3.py,v 1.4 2007/10/29 14:29:47 mato Exp $
# =============================================================================
# CVS tag $Name:  $, version $Revision: 1.4 $
# =============================================================================
## @file
#
# Simple example which illustrate the usage of useful
# algorithm  base class for N-Tuple manipulations
#
# @author Vanya BELYAEV ibelyaev@physics.syr.edu
# @date 2007-01-24
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
__author__ = 'Vanya BELYAEV ibelyaev@physics.syr.edu'
# =============================================================================

import GaudiPython, math

Rndm        = GaudiPython.gbl.Rndm
Math        = GaudiPython.gbl.ROOT.Math
SUCCESS     = GaudiPython.SUCCESS
Gaudi       = GaudiPython.gbl.Gaudi

GaudiPython.loaddict('STLRflx')
GaudiPython.loaddict('STLAddRflx')
GaudiPython.loaddict('MathRflx')
GaudiPython.loaddict('MathAddRflx')

vct1        = GaudiPython.gbl.vector('double')

GaudiPython.loaddict('CLHEPRflx')
vct2        = GaudiPython.gbl.CLHEP.HepVector

from   GaudiPython.GaudiAlgs   import TupleAlgo


# =============================================================================
## @class TupleEx3
#  Simple algorithm for advanced N-Tuple columns
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2007-01-24
class TupleEx3(TupleAlgo) :
    """
    Simple algorithm for advanced (matrix&array) N-Tuple columns
    """

    ## standard constructor
    def __init__ ( self , name = 'TupleEx3' ) :
        """ Constructor """
        TupleAlgo.__init__( self , name )

    ## the main execution method
    def execute( self ) :
        """ The major method 'execute', it is invoked for each event """

        gauss = Rndm.Numbers ( self.randSvc() , Rndm.Gauss ( 0.0 , 1.0 ) )
        flat  = Rndm.Numbers ( self.randSvc() , Rndm.Flat  ( -10 , 10  ) )
        breit = Rndm.Numbers ( self.randSvc() , Rndm.BreitWigner  ( 0.0 , 1.0  ) )

        ## N-tuple with farrays
        tup = self.nTuple('farrays', 'N-tuple with farrays')

        for i in range(0,20) :

            # std::vector<double>

            # prepare some data array (std::vector<double>)
            v1=vct1()
            n=long(50+2*flat())
            # fill it with some random data
            for j in range(0,n) : v1.push_back( gauss() )
            #fill N-tuple
            tup.farray('gauss',v1,'len1',150)

            # prepare some data array (CLHEP::HepVector<double>)
            n=long(50+2*flat())
            v2=vct2(n)
            #fill N-tuple
            tup.farray('breit',v2,'len2',150)

            # commit the row
            tup.write()

        ## N-tuple with arrays
        tup = self.nTuple('arrays', 'N-tuple with arrays')
        for i in range(0,20) :

            # std::vector<double>

            # prepare some data array (std::vector<double>)
            v1=vct1(30,1.0)
            #fill N-tuple
            tup.array('gauss',v1)

            # use CLHEP::HepVector
            v2=vct2( 30 )
            tup.array('breit',v2)

            # use Gaudi::Vector2
            v= Gaudi.Vector2()
            tup.array ( 'v2' , v ) ;

            # use Gaudi::Vector3
            v= Gaudi.Vector3()
            tup.array ( 'v3' , v ) ;

            # use Gaudi::Vector4
            v= Gaudi.Vector4()
            tup.array ( 'v4' , v ) ;

            # use Gaudi::Vector5
            v= Gaudi.Vector5()
            tup.array ( 'v5' , v ) ;

            # use Gaudi::Vector6
            v= Gaudi.Vector6()
            tup.array ( 'v6' , v ) ;

            # use Gaudi::Vector7
            v= Gaudi.Vector7()
            tup.array ( 'v7' , v ) ;

            # use Gaudi::Vector8
            v= Gaudi.Vector8()
            tup.array ( 'v8' , v ) ;

            # use Gaudi::Vector9
            v= Gaudi.Vector9()
            tup.array ( 'v9' , v ) ;

            # commit the row
            tup.write()

        ## N-tuple with fixed square matrices
        tup = self.nTuple('square', 'N-tuple with square matrices')
        for i in range(0,20) :

            # make a matrice
            m=Gaudi.Matrix2x2()
            tup.matrix ( "m2" , m )

            # make a matrice
            m=Gaudi.Matrix3x3()
            tup.matrix ( "m3" , m )

            # make a matrice
            m=Gaudi.Matrix4x4()
            tup.matrix ( "m4" , m )

            # make a matrice
            m=Gaudi.Matrix5x5()
            # fill n-tuple
            tup.matrix ( "m5" , m )

            # make a matrice
            m=Gaudi.Matrix6x6()
            # fill n-tuple
            tup.matrix ( "m6" , m )

            # make a matrice
            m=Gaudi.Matrix7x7()
            tup.matrix ( "m7" , m )

            # make a matrice
            m=Gaudi.Matrix8x8()
            # fill n-tuple
            tup.matrix ( "m8" , m )

            # make a matrice
            m=Gaudi.Matrix9x9()
            tup.matrix ( "m9" , m )

            # commit the row
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

    gaudi.ExtSvc += ["NTupleSvc" , 'ChronoStatSvc']

    ntSvc = gaudi.service('NTupleSvc')
    ntSvc.Output = [ "MYLUN DATAFILE='TupleEx3.root' OPT='NEW' TYP='ROOT'" ]

    gaudi.config()

    gaudi.DLLs = [ 'GaudiAlg', 'RootHistCnv', ]

    alg = TupleEx3()
    gaudi.setAlgorithms( [alg] )

    # configure proeprties
    alg.NTupleLUN = 'MYLUN'

    return SUCCESS


# =============================================================================
## The actual job execution
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2007-01-24
if '__main__' == __name__ :
    print __doc__
    gaudi = GaudiPython.AppMgr()
    configure( gaudi )
    gaudi.run(10)

# =============================================================================
# The END
# =============================================================================

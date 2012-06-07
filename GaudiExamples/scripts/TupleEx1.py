#!/usr/bin/env python
# =============================================================================
# $Id: TupleEx1.py,v 1.12 2007/10/29 14:29:47 mato Exp $
# =============================================================================
# CVS tag $Name:  $ , version $Revision: 1.12 $
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

SUCCESS = GaudiPython.SUCCESS

# random numbewrs
Rndm    = GaudiPython.gbl.Rndm
Numbers = Rndm.Numbers

from   GaudiPython.GaudiAlgs   import TupleAlgo, mapvct

# =============================================================================
## Primitive function which transform arbitrary sequence  into
#  GaudiPython.Vector ( std::vector<double> )
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
def vct( sequence ) :
    """
    Primitive function which transform arbitrary sequence  into
    GaudiPython.Vector ( std::vector<double> )
    """
    result = GaudiPython.gbl.GaudiPython.Vector()
    if   hasattr( sequence , '__len__' ) : result.reserve ( len(sequence)   )
    elif hasattr( sequence , 'size'    ) : result.reserve ( sequence.size() )

    for item in sequence : result.push_back( item )
    return result

# =============================================================================
## @class TupleEx1
#  Simple algorithm which book&fill 3 histograms
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
class TupleEx1(TupleAlgo) :
    """
    Simple algorithm which implicitely book&fill N-Tuples
    """
    ## the main executiomethod
    def execute( self ) :
        """
        The major method 'execute', it is invoked for each event
        """

        rSvc    = self.randSvc()
        gauss   = Numbers ( rSvc , Rndm.Gauss        (   0.0 ,   1.0  ) )
        flat    = Numbers ( rSvc , Rndm.Flat         ( -10   ,  10    ) )
        expo    = Numbers ( rSvc , Rndm.Exponential  (   1.0          ) )
        breit   = Numbers ( rSvc , Rndm.BreitWigner  (   0.0 ,   1.0  ) )
        poisson = Numbers ( rSvc , Rndm.Poisson      (   2.0          ) )
        binom   = Numbers ( rSvc , Rndm.Binomial     (   8   ,   0.25 ) )

        # =====================================================================
        # primitive row-wise n-tuple
        # =====================================================================
        tuple1  = self.nTuple ( 1 , "Trivial Row-Wise Tuple" , 42 )

        # fill N-Tuple with double/float numbers:
        tuple1 . column ( 'gauss' , gauss () )
        tuple1 . column ( 'flat'  , flat  () )
        tuple1 . column ( 'expo'  , expo  () )
        tuple1 . column ( 'breit' , breit () )

        # fill N-Tuple with integer numbers:
        tuple1 . column ( 'poiss1' , int( poisson () ) )
        tuple1 . column ( 'binom1' , int( binom   () ) )

        # fill N-Tuple with long long numbers:
        tuple1 . column_ll  ( 'poiss2' , int( poisson () ) )
        tuple1 . column_ll  ( 'binom2' , int( binom   () ) )
        
        # fill N-Tuple with unsigned long long numbers:
        tuple1 . column_ull ( 'poiss3' , int( poisson () ) )
        tuple1 . column_ull ( 'binom3' , int( binom   () ) )

        # fill N-Tuple with "reduced" integer numbers:
        tuple1 . column ( 'poiss4' , int( poisson () ) , 0 , 14 )
        tuple1 . column ( 'binom4' , int( binom   () ) , 0 , 14 )

        # fill N-Tuple with "boolean" numbers:
        tuple1 . column ( "poisb" ,  poisson () > 1.0 )

        # commit the row
        tuple1 . write()

        # =====================================================================
        # the same n-tuple but column-wise
        # =====================================================================
        tuple2  = self.nTuple ( 2 , "Trivial Column-Wise Tuple" )

        # fill N-Tuple with double/float numbers:
        tuple2 . column ( 'gauss' , gauss () )
        tuple2 . column ( 'flat'  , flat  () )
        tuple2 . column ( 'expo'  , expo  () )
        tuple2 . column ( 'breit' , breit () )

        # fill N-Tuple with integer numbers:
        tuple2 . column ( 'poiss' , int( poisson () ) )
        tuple2 . column ( 'binom' , int( binom   () ) )
        # fill N-Tuple with "reduced" integer numbers:
        tuple2 . column ( 'poiss' , int( poisson () ) , 0 , 14 )
        tuple2 . column ( 'binom' , int( binom   () ) , 0 , 14 )

        # fill N-Tuple with "boolean" numbers:
        tuple2 . column ( "poisb" ,  poisson () > 1.0 )

        # commit the row
        tuple2 . write()

        # =====================================================================
        # book and fill Column-wise NTuple with "fixed"-size arrays/vectors
        # =====================================================================
        tuple3 = self.nTuple ( 3 , "Fixed-size arrays/vectors" )

        tuple3.array ( 'arflat' , vct( [ flat () for i in xrange(0,50) ] ) )
        tuple3.array ( 'arexpo' , vct( [ expo () for i in xrange(0,62) ] ) )
        tuple3.array ( 'argau'  , vct( [ gauss() for i in xrange(0,42) ] ) )
        t=tuple([ gauss() for i in xrange(0,42) ])
        tuple3.array ( 'argau2' , vct( t ) )

        tuple3.write()

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
    ntSvc.Output = [ "MYLUN DATAFILE='TupleEx1.root' OPT='NEW' TYP='ROOT'" ]

    gaudi.config()

    gaudi.DLLs = [ 'GaudiAlg', 'RootHistCnv', ]

    alg = TupleEx1('TupleEx1')
    gaudi.setAlgorithms( [alg] )

    ## configure the properties
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

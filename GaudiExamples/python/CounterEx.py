#!/usr/bin/env python
# =============================================================================
# $Id: CounterEx.py,v 1.8 2007/10/29 14:29:47 mato Exp $
# =============================================================================
"""
*******************************************************************************
*                                                                             *
* Simple example which illustrates the usage of the useful base class         *
* GaudiAlgo (python version of C++ GaudiAlgorithm) for "easy" manipulations   *
* with 'counters'                                                             *
*                                                                             *
* The example is equivalent to C++ example 'CounterEx', see                   *
* $GAUDIEXAMPLESROOT/src/CounterEx/*.cpp and                                  *
* $GAUDIEXAMPLESROOT/options/CounterEx.opts                                   *
*                                                                             *
*******************************************************************************
"""
# =============================================================================
__author__ = 'Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr'
# =============================================================================

import GaudiPython

from   GaudiPython.GaudiAlgs   import GaudiAlgo

Rndm    = GaudiPython.gbl.Rndm
Numbers = Rndm.Numbers
SUCCESS = GaudiPython.SUCCESS

Numbers.__call__ = Numbers.shoot

# =============================================================================
# Simple algorithm which manipulates with counters
# =============================================================================
class Counter(GaudiAlgo) :
    """ Simple algorithm which manipulates with counters """
    def __init__ ( self , name = 'Counter' ) :
        """ Constructor """
        GaudiAlgo.__init__( self , name )

    def execute( self ) :
        """ The major method 'execute', it is invoked for each event """

        executed = self.counter('executed')
        executed += 1.

        gauss   = Numbers( self.randSvc() , Rndm.Gauss   ( 0.0 ,1.0 ) )
        poisson = Numbers( self.randSvc() , Rndm.Poisson ( 5.0      ) )

        # 'accuulate gauss'
        value = gauss.shoot()

        g1 = self.counter('gauss')
        g2 = self.counter('g2')

        g1 += value
        g2 += value * value

        if 0 < value :
            gp = self.counter('Gpos')
            gp += 1.
        else :
            gn = self.counter('Gneg')
            gn += 1.

        stat1 = self.counter('NG')
        stat2 = self.counter('G')
        for i in range ( 0, int( poisson() ) ) :
            stat1 += 1.
            stat2 += gauss()

        stat3  = self.counter('eff')
        stat3 += value>0

        # print statistics every 1000 events
        executed = self.counter('executed')
        prnt = int( executed.flag() )
        if 0 == prnt%1000 :
            print " Event number %s " % prnt
            self.printStat()
            bc = self.counter('eff')
            line = "(%s += %s)%s"%(bc.eff()*100,bc.effErr()*100,'%')
            print ' Efficiency (binomial counter "eff"): %s'%line

        return SUCCESS


# =============================================================================
# job configuration
# =============================================================================
def configure( gaudi = None  ) :
    """ Configuration of the job """

    if not gaudi : gaudi = GaudiPython.AppMgr()

    gaudi.JobOptionsType       = 'NONE'
    gaudi.EvtSel               = 'NONE'

    gaudi.config()

    alg = Counter()
    gaudi.setAlgorithms( [alg] )

    # configure the properties
    alg.StatPrint = True

    return SUCCESS

# =============================================================================
# The actual job excution
# =============================================================================
if '__main__' == __name__ :
    print __doc__ , __author__
    gaudi = GaudiPython.AppMgr()
    configure( gaudi )
    gaudi.run(5400)

# =============================================================================
# The END
# =============================================================================

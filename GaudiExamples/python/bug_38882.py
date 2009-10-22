#!/usr/bin/env python
# =============================================================================
# $Id: bug_38882.py,v 1.1 2008/07/15 16:00:51 marcocle Exp $
# =============================================================================

# =============================================================================
__author__ = 'Marco Clemencic'
# =============================================================================

import GaudiPython

from GaudiPython.GaudiAlgs import GaudiAlgo

SUCCESS = GaudiPython.SUCCESS

# =============================================================================
# Simple algorithm which book&fill 3 histograms
# =============================================================================
class TestAlg(GaudiAlgo) :
    """ Simple algorithm that prints a message during execute """
    def __init__ ( self , name ) :
        """ Constructor """
        GaudiAlgo.__init__( self , name )

    def execute( self ) :
        """ The main method 'execute', it is invoked for each event """
        print "=== %s Execute ===" % self.name()
        return SUCCESS

# =============================================================================
# job configuration
# =============================================================================
def configure( gaudi = None  ) :
    """ Configuration of the job """

    if not gaudi : gaudi = GaudiPython.AppMgr()

    gaudi.JobOptionsType       = 'NONE'
    gaudi.EvtSel               = 'NONE'
    gaudi.HistogramPersistency = 'NONE'

    gaudi.config()

    gaudi.initialize()

    alg = TestAlg('bug_38882_test_alg')
    gaudi.setAlgorithms( [alg] )

    return SUCCESS


# =============================================================================
# The actual job excution
# =============================================================================
if '__main__' == __name__ :
    gaudi = GaudiPython.AppMgr()
    configure( gaudi )
    gaudi.run(1)

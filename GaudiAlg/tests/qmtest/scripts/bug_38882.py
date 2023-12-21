#!/usr/bin/env python3
#####################################################################################
# (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# =============================================================================

# =============================================================================

__author__ = "Marco Clemencic"
# =============================================================================

from GaudiAlg.Algs import GaudiAlgo

import GaudiPython

SUCCESS = GaudiPython.SUCCESS

# =============================================================================
# Simple algorithm which book&fill 3 histograms
# =============================================================================


class TestAlg(GaudiAlgo):
    """Simple algorithm that prints a message during execute"""

    def __init__(self, name):
        """Constructor"""
        GaudiAlgo.__init__(self, name)

    def execute(self):
        """The main method 'execute', it is invoked for each event"""
        print("=== %s Execute ===" % self.name())
        return SUCCESS


# =============================================================================
# job configuration
# =============================================================================


def configure(gaudi=None):
    """Configuration of the job"""

    if not gaudi:
        gaudi = GaudiPython.AppMgr()

    gaudi.JobOptionsType = "NONE"
    gaudi.EvtSel = "NONE"
    gaudi.HistogramPersistency = "NONE"

    gaudi.config()

    gaudi.initialize()

    alg = TestAlg("bug_38882_test_alg")
    gaudi.setAlgorithms([alg])

    return SUCCESS


# =============================================================================
# The actual job excution
# =============================================================================
if "__main__" == __name__:
    gaudi = GaudiPython.AppMgr()
    configure(gaudi)
    gaudi.run(1)

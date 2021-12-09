#!/usr/bin/env python
#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# =============================================================================
"""
*******************************************************************************
*                                                                             *
* Simple example which illustrate the usage of useful algorithm  base class   *
* HistoAlgo (python version of C++ GaudiHistoAlg) for "easy" histogramming.   *
* It is an extension of HistoEx module, it provides similar algorithm, but    *
* with explicit histogram manipulation trhrough explicit book and fill        *
*                                                                             *
*******************************************************************************
"""
from __future__ import print_function

# =============================================================================
__author__ = "Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr"
# =============================================================================

from GaudiPython.GaudiAlgs import SUCCESS, HistoAlgo

# =============================================================================
# Simple algorithm which book&fill 3 histograms
# =============================================================================


class HistoEx1(HistoAlgo):
    """Simple algorithm which explicitely book&fill three histograms"""

    def __init__(self, name):
        """Constructor"""
        HistoAlgo.__init__(self, name)

    def initialize(self):
        """Initialization, initialize the base class and book histograms"""
        status = HistoAlgo.initialize(self)
        if status.isFailure():
            return status

        self.h1 = self.book1D(" 1D histo ", 0, 20, 20)
        self.h2 = self.book2D(" 2D histo ", 0, 20, 20, 0, 20, 20)
        self.h3 = self.book3D(" 3D histo ", 0, 20, 20, 0, 20, 20, 0, 20, 20)

        return SUCCESS

    def execute(self):
        """The major method 'execute', it is invoked for each event"""

        for i in range(0, 10):
            self.h1.fill(i, 0.166)
            for j in range(0, 10):
                self.h2.fill(i, j)
                for k in range(0, 10):
                    self.h3.fill(i, j, k)

        return SUCCESS


# =============================================================================
# job configuration
# =============================================================================
def configure(gaudi=None):
    """Configuration of the job"""

    import HistoEx

    if not gaudi:
        from GaudiPython.Bindings import AppMgr

        gaudi = AppMgr()

    HistoEx.configure(gaudi)

    alg = HistoEx1("HistoEx1")
    gaudi.addAlgorithm(alg)

    alg.HistoPrint = True

    return SUCCESS


# =============================================================================
# The actual job excution
# =============================================================================
if "__main__" == __name__:
    print(__doc__ + __author__)

    from GaudiPython.Bindings import AppMgr

    gaudi = AppMgr()

    configure(gaudi)

    gaudi.run(20)
    import GaudiPython.HistoUtils

    alg = gaudi.algorithm("HistoEx1")
    histos = alg.Histos()
    for key in sorted(histos):
        histo = histos[key]
        if hasattr(histo, "dump"):
            print(histo.dump(80, 20, True))

# =============================================================================

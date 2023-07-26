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
"""
*******************************************************************************
*                                                                             *
* Simple example which illustrate the usage of useful algorithm  base class   *
* HistoAlgo (python version of C++ GaudiHistoAlg) for "easy" histogramming.   *
*                                                                             *
*******************************************************************************
"""
# =============================================================================
__author__ = "Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr"
# =============================================================================

from GaudiAlg.Algs import SUCCESS, HistoAlgo

# =============================================================================
# Simple algorithm which book&fill 3 histograms
# =============================================================================


class HistoEx(HistoAlgo):
    """Simple algorithm which implicitely book&fill three histograms"""

    def __init__(self, name):
        """Constructor"""
        HistoAlgo.__init__(self, name)

    def execute(self):
        """The major method 'execute', it is invoked for each event"""
        for i in range(0, 10):
            self.plot1D(i, " 1D histo ", 0, 20, 20)
            for j in range(0, 10):
                self.plot2D(i, j, " 2D histo ", 0, 20, 0, 20, 20, 20)
                for k in range(0, 10):
                    self.plot3D(i, j, k, " 3D histo ", 0, 20, 0, 20, 0, 20, 20, 20, 20)

        return SUCCESS


# =============================================================================
# job configuration
# =============================================================================
def configure(gaudi=None):
    """Configuration of the job"""

    if not gaudi:
        from GaudiPython.Bindings import AppMgr

        gaudi = AppMgr()

    gaudi.JobOptionsType = "NONE"
    gaudi.EvtSel = "NONE"
    gaudi.HistogramPersistency = "ROOT"

    gaudi.config()

    gaudi.DLLs = [
        "GaudiAlg",
        "RootHistCnv",
    ]

    alg = HistoEx("HistoEx")
    gaudi.setAlgorithms([alg])
    alg.HistoPrint = True

    hsvc = gaudi.service("HistogramPersistencySvc")
    hsvc.OutputFile = "histoex.root"

    # This does not harm and tests bug #50389
    _ = gaudi.algorithm("HistoEx")

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

    import GaudiPython.HistoUtils  # noqa: F401 (adds dump method)

    alg = gaudi.algorithm("HistoEx")
    histos = alg.Histos()
    for key in sorted(histos):
        histo = histos[key]
        if hasattr(histo, "dump"):
            print(histo.dump(80, 20, True))

# =============================================================================
# The END
# =============================================================================

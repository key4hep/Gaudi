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
# @file
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
from __future__ import print_function

# =============================================================================
__author__ = "Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr"
# =============================================================================

import math

from GaudiPython.Bindings import gbl as cpp

Rndm = cpp.Rndm

from GaudiPython.GaudiAlgs import SUCCESS, TupleAlgo

# =============================================================================
# @class TupleEx
#  Simple algorithm which books&fills three histograms
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26


class TupleEx(TupleAlgo):
    """
    Simple algorithm which implicitely book&fill N-Tuples
    """

    # standard constructor

    def __init__(self, name="TupleEx", **args):
        """Constructor"""
        TupleAlgo.__init__(self, name, **args)

    # the main execution method
    def execute(self):
        """
        The major method 'execute', it is invoked for each event
        """

        gauss = Rndm.Numbers(self.randSvc(), Rndm.Gauss(0.0, 1.0))

        tup = self.nTuple("My trivial N-tuple")
        for i in range(0, 100):
            tup.column("a", math.sin(i))
            tup.column("b", math.cos(i))
            tup.column("c", math.tan(i))
            tup.column("g", gauss())
            tup.write()

        return SUCCESS


# =============================================================================
# job configuration
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
def configure(gaudi=None):
    """Configuration of the job"""

    if not gaudi:
        from GaudiPython.Bindings import AppMgr

        gaudi = AppMgr()

    gaudi.JobOptionsType = "NONE"
    gaudi.EvtSel = "NONE"
    gaudi.HistogramPersistency = "ROOT"

    gaudi.ExtSvc += ["NTupleSvc"]

    ntSvc = gaudi.service("NTupleSvc")
    ntSvc.Output = ["MYLUN DATAFILE='TupleEx.root' OPT='NEW' TYP='ROOT'"]

    gaudi.config()

    gaudi.DLLs = [
        "GaudiAlg",
        "RootHistCnv",
    ]

    alg = TupleEx(
        # configure the property
        NTupleLUN="MYLUN"
    )

    gaudi.setAlgorithms([alg])

    return SUCCESS


# =============================================================================
# The actual job excution
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-11-26
if "__main__" == __name__:
    print(__doc__)
    from GaudiPython.Bindings import AppMgr

    gaudi = AppMgr()
    configure(gaudi)
    gaudi.run(20)

# =============================================================================
# The END
# =============================================================================

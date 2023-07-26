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
* Simple example which illustrate the usage of useful                         *
* algorithm  base class for N-Tuple manipulations                             *
*                                                                             *
*******************************************************************************
"""
# =============================================================================
__author__ = "Vanya BELYAEV Ivan.Belyaev@lapp.in2p3.fr"
# =============================================================================

import GaudiPython

SUCCESS = GaudiPython.SUCCESS

# =============================================================================
# job configuration
# =============================================================================


def configure(gaudi=None):
    """Configuration of the job"""

    gaudi.HistogramPersistency = "ROOT"

    gaudi.DLLs += [
        "GaudiAlg",
        "RootHistCnv",
    ]
    gaudi.ExtSvc += ["TagCollectionSvc/EvtTupleSvc"]

    evtSel = gaudi.evtSel()
    evtSel.open("PFN:EvtTags1.root", collection="Fill/COL1", sel="binom==6")

    gaudi.config()

    return SUCCESS


# =============================================================================
# The actual job excution
# =============================================================================
if "__main__" == __name__:
    print(__doc__)
    # configuration (options)
    from Configurables import ApplicationMgr, FileCatalog, GaudiPersistency

    GaudiPersistency()
    ApplicationMgr().ExtSvc.append(
        FileCatalog(Catalogs=["xmlcatalog_file:EvtColsEx.xml"])
    )
    # execution
    gaudi = GaudiPython.AppMgr()
    configure(gaudi)
    gaudi.run(-1)
    gaudi.exit()

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
## @file DumpParticleProperties.py
#  Demo-file for interactive work with new Particle Property Service
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date 2010-10-22
# =============================================================================
"""Trivial script to dump the table of Particle Properties
"""

# =============================================================================
__author__ = "Vanya BELYAEV Ivan.Belyaev@nikhef.nl"
# =============================================================================


def dump():
    import PartProp.PartPropAlg

    # The following is misunderstood by flake8 - the import is needed as it
    # has necessary side effects
    import PartProp.Service  # noqa: F401
    from Configurables import MessageSvc

    MessageSvc(OutputLevel=6)

    from GaudiPython.Bindings import AppMgr

    gaudi = AppMgr()

    gaudi.initialize()

    pps = gaudi.ppSvc()

    print(pps.all())


# =============================================================================
if "__main__" == __name__:
    dump()

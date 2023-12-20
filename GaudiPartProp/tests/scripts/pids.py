#!/usr/bin/env python
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
## @file PartPropSvc.py
#  Demo-file for interactive work with new Particle Property Service
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date 2008-12-02
# =============================================================================
"""
Demo-file for interactive work with new Particle Property Service
"""

__author__ = "Vanya BELYAEV Ivan.Belyaev@nikhef.nl"
from pathlib import Path

# The following lines are misunderstood by flake8 - the imports are needed as
# they have necessary side effects
import GaudiPartProp.PartPropAlg  # noqa: F401
import GaudiPartProp.Service  # noqa: F401

# from DDDB.CheckDD4Hep import UseDD4Hep
# if UseDD4Hep:
#     # use a predefined particle table file
from Configurables import Gaudi__ParticlePropertySvc

## test the nodes
from GaudiPartProp.Nodes import Gaudi
from GaudiPython.Bindings import AppMgr

Gaudi__ParticlePropertySvc(
    ParticlePropertiesFile=str(
        Path(__file__).parent / "ParticleTable.dddb-20200424.txt"
    )
)

gaudi = AppMgr()
gaudi.initialize()


## simple test-function
def test():
    """Simple function for the test"""
    pps = gaudi.gaudiPartProp()

    ##  get all known pids
    all = pps.get(lambda x: True)

    pids = set()
    for p in all:
        pids.add(p.particleID().pid())

    ## add "special" values
    pids.add(10022)
    pids.add(20022)
    ##
    pids.add(30343)
    pids.add(30353)
    pids.add(30363)
    ##
    pids.add(1009002)
    pids.add(1000542)
    pids.add(1000522)
    pids.add(1000039)
    pids.add(3100021)
    pids.add(480000000)
    pids.add(-990000000)

    for p in sorted(pids):
        pp = Gaudi.ParticleID(p)

        print(
            "({})".format(
                ", ".join(
                    str(x)
                    for x in (
                        pp.pid(),
                        pp.abspid(),
                        pp.isValid(),
                        pp.isSM(),
                        pp.isMeson(),
                        pp.isBaryon(),
                        pp.isHadron(),
                        pp.isLepton(),
                        pp.isNucleus(),
                        pp.isQuark(),
                        pp.hasQuarks(),
                        pp.hasDown(),
                        pp.hasUp(),
                        pp.hasStrange(),
                        pp.hasCharm(),
                        pp.hasBottom(),
                        pp.hasTop(),
                        pp.hasBottomPrime(),
                        pp.hasTopPrime(),
                        pp.threeCharge(),
                        pp.jSpin(),
                        pp.sSpin(),
                        pp.lSpin(),
                        pp.Z(),
                        pp.A(),
                        pp.nLambda(),
                        pp.fundamentalID(),
                        pp.extraBits(),
                    )
                )
            )
        )


if "__main__" == __name__:
    test()

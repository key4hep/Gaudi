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
## @file newparts.py
#  Simple test for adding new particle
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date 2018-07-12
# =============================================================================
""" Simple test for adding new particle
"""

# =============================================================================
__author__ = "Vanya BELYAEV Ivan.Belyaev@itep.ru"
from pathlib import Path

from Configurables import Gaudi__ParticlePropertySvc

import GaudiPartProp.decorators

# =============================================================================
# The following lines are misunderstood by flake8 - the imports are needed as they
# have necessary side effects
import GaudiPartProp.PartPropAlg  # noqa: F401
import GaudiPartProp.Service  # noqa: F401
from GaudiKernel.SystemOfUnits import GeV, second
from GaudiPython.Bindings import AppMgr

# =============================================================================
## test the nodes
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

    try:
        # If we have no ParticleProperties this isn't going to work
        c = pps.find("chi_c1(1P)")

        ## redefine properties
        line1 = "%s 0 %d 0.0 %g %20.12g %s %d %g " % (
            c.name(),
            c.pid().pid(),
            c.mass() / GeV,
            c.lifetime() / second,
            c.evtGen(),
            c.pythia(),
            c.maxWidth() / GeV,
        )
        line2 = "%s 0 %d 0.0 %g %20.12g %s %d %g " % (
            c.name(),
            c.pid().pid(),
            c.mass() / GeV,
            -c.width() / GeV,
            c.evtGen(),
            c.pythia(),
            c.maxWidth() / GeV,
        )

        pps.Particles = [line1]
        pps.Particles = [line2]
    except ReferenceError:
        pass


# =============================================================================
if "__main__" == __name__:
    test()

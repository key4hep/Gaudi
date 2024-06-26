#####################################################################################
# (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiTesting import GaudiExeTest


class Test(GaudiExeTest):
    command = ["../../tests/scripts/newparts.py"]

    test_block1 = GaudiExeTest.find_reference_block("""
        Gaudi::Particle...SUCCESS  New/updated particles (from "Particles" property)
         ------------------------------------------------------------------------------------------------------------------------------------------------------------------
         | #    |        Name       |     PdgID    |   Q  |        Mass       |    (c*)Tau/Gamma  |  MaxWidth  |        EvtGen        |  PythiaID  |     Antiparticle     |
         ------------------------------------------------------------------------------------------------------------------------------------------------------------------
         | 582  | chi_c1(1P)        |        20443 |   0  |       3.51067 GeV |     839.99989 keV |      0     |        chi_c1        |    20443   |        self-cc       |
         ------------------------------------------------------------------------------------------------------------------------------------------------------------------
        """)
    test_block2 = GaudiExeTest.find_reference_block("""
        Gaudi::Particle...SUCCESS  New/updated particles (from "Particles" property)
         ------------------------------------------------------------------------------------------------------------------------------------------------------------------
         | #    |        Name       |     PdgID    |   Q  |        Mass       |    (c*)Tau/Gamma  |  MaxWidth  |        EvtGen        |  PythiaID  |     Antiparticle     |
         ------------------------------------------------------------------------------------------------------------------------------------------------------------------
         | 582  | chi_c1(1P)        |        20443 |   0  |       3.51067 GeV |     839.99989 keV |      0     |        chi_c1        |    20443   |        self-cc       |
         ------------------------------------------------------------------------------------------------------------------------------------------------------------------
        """)

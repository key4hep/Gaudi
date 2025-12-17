#####################################################################################
# (c) Copyright 2024-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiTesting import GaudiExeTest


class TestCustomIncidentAlg(GaudiExeTest):
    command = ["gaudirun.py", "-v"]
    returncode = 16

    def options(self):
        from Configurables import ApplicationMgr, GaudiPersistency
        from Configurables import GaudiTesting__CustomIncidentAlg as cia

        GaudiPersistency()

        app = ApplicationMgr(  # noqa: F841
            TopAlg=[cia(EventCount=3, Incident="CorruptedInputFile")],
            EvtSel="NONE",
            EvtMax=5,
        )

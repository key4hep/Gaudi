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


class TestAlgorithmDestructorMT(GaudiExeTest):
    command = ["gaudirun.py", "-v"]

    def options(self):
        from Configurables import (
            AlgResourcePool,
            ApplicationMgr,
            HiveSlimEventLoopMgr,
            HiveWhiteBoard,
        )
        from Configurables import GaudiTesting__DestructorCheckAlg as dca

        from Gaudi.Configuration import INFO

        algResourcePool = AlgResourcePool(OutputLevel=INFO)
        slimeventloopmgr = HiveSlimEventLoopMgr(
            SchedulerName="AvalancheSchedulerSvc", OutputLevel=INFO
        )
        whiteboard = HiveWhiteBoard("EventDataSvc")

        ApplicationMgr(
            TopAlg=[dca("TopAlg")],
            EvtSel="NONE",
            EvtMax=2,
            EventLoop=slimeventloopmgr,
            ExtSvc=[algResourcePool, whiteboard],
        )

    test_check_line = GaudiExeTest.find_reference_block(
        "Destructor of TopAlg",
    )

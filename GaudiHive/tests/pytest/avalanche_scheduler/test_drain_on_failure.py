#####################################################################################
# (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations         #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,       #
# copied verbatim in the file "LICENSE".                                              #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities         #
# granted to it by virtue of its status as an Intergovernmental Organization           #
# or submit itself to any jurisdiction.                                              #
#####################################################################################
from GaudiTesting import GaudiExeTest


class Test(GaudiExeTest):
    """An event failure must drain other events before algorithms are finalized."""

    command = ["gaudirun.py"]
    returncode = 1
    timeout = 30

    def options():
        from Configurables import (
            ApplicationMgr,
            AvalancheSchedulerSvc,
            CPUCruncher,
            CPUCrunchSvc,
            HiveSlimEventLoopMgr,
            HiveWhiteBoard,
        )

        from Gaudi.Configuration import WARNING

        CPUCrunchSvc(shortCalib=True)
        # More slots than threads leave work outstanding when an event fails.
        work = CPUCruncher(
            "Work", Cardinality=15, avgRuntime=0.02, varRuntime=0, FailNEvents=7
        )
        AvalancheSchedulerSvc(ThreadPoolSize=10, OutputLevel=WARNING)
        ApplicationMgr(
            TopAlg=[work],
            EvtMax=30,
            EvtSel="NONE",
            HistogramPersistency="NONE",
            ExtSvc=[HiveWhiteBoard("EventDataSvc", EventSlots=15)],
            EventLoop=HiveSlimEventLoopMgr(),
        )

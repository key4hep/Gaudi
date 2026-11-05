#####################################################################################
# (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations      #
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
    """
    A top algorithm whose initialize() returns FAILURE (without throwing) makes
    the job fail cleanly when it is created by the AlgResourcePool.
    """

    command = ["gaudirun.py"]
    returncode = 1

    def options(self):
        from Configurables import (
            AlgResourcePool,
            ApplicationMgr,
            HiveSlimEventLoopMgr,
            HiveWhiteBoard,
        )
        from Configurables import GaudiTesting__CustomIncidentAlg as CustomIncidentAlg

        slimeventloopmgr = HiveSlimEventLoopMgr(SchedulerName="AvalancheSchedulerSvc")
        ApplicationMgr(
            # without the Incident property, initialize() returns FAILURE
            TopAlg=[CustomIncidentAlg("InitFailAlg")],
            EvtSel="NONE",
            EvtMax=1,
            EventLoop=slimeventloopmgr,
            ExtSvc=[AlgResourcePool(), HiveWhiteBoard("EventDataSvc")],
        )

    test_check_line = GaudiExeTest.find_reference_block(
        """
        AlgorithmManager     ERROR Failed to initialize algorithm: [InitFailAlg]
        AlgResourcePool      ERROR Unable to create or initialize Algorithm: GaudiTesting::CustomIncidentAlg/InitFailAlg
        """
    )

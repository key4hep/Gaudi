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


class TestGlobalDefaults(GaudiExeTest):
    command = ["gaudirun.py"]

    def options(self):
        from Configurables import HelloWorld

        from Gaudi.Configuration import ApplicationMgr, JobOptionsSvc

        app = ApplicationMgr(EvtSel="NONE", EvtMax=1)

        app.TopAlg = [HelloWorld("alg{}".format(i)) for i in range(10)]
        app.TopAlg[5].OutputLevel = 4

        JobOptionsSvc(
            GlobalDefaults=[
                (r".*[135]\.OutputLevel$", "1"),
                (r".*[24]\.OutputLevel$", "2"),
            ]
        )

    test_check_line = GaudiExeTest.find_reference_block(
        """
        alg0                  INFO initializing....
        alg1                 DEBUG Property update for OutputLevel : new value = 1
        alg1               VERBOSE ServiceLocatorHelper::service: found service EventDataSvc
        alg1               VERBOSE ServiceLocatorHelper::service: found service TimelineSvc
        alg1                  INFO initializing....
        alg1               VERBOSE ServiceLocatorHelper::service: found service AlgExecStateSvc
        alg1                 DEBUG input handles: 0
        alg1                 DEBUG output handles: 0
        alg2                 DEBUG Property update for OutputLevel : new value = 2
        alg2                  INFO initializing....
        alg2                 DEBUG input handles: 0
        alg2                 DEBUG output handles: 0
        alg3                 DEBUG Property update for OutputLevel : new value = 1
        alg3               VERBOSE ServiceLocatorHelper::service: found service EventDataSvc
        alg3               VERBOSE ServiceLocatorHelper::service: found service TimelineSvc
        alg3                  INFO initializing....
        alg3               VERBOSE ServiceLocatorHelper::service: found service AlgExecStateSvc
        alg3                 DEBUG input handles: 0
        alg3                 DEBUG output handles: 0
        alg4                 DEBUG Property update for OutputLevel : new value = 2
        alg4                  INFO initializing....
        alg4                 DEBUG input handles: 0
        alg4                 DEBUG output handles: 0
        alg6                  INFO initializing....
        alg7                  INFO initializing....
        alg8                  INFO initializing....
        alg9                  INFO initializing....
        """
    )

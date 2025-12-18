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


class TestSignalHandler(GaudiExeTest):
    command = ["gaudirun.py"]
    environment = [
        "JOBOPTSEARCHPATH=$ENV_PROJECT_SOURCE_DIR/GaudiTestSuite/options:$ENV_PROJECT_SOURCE_DIR/Gaudi/GaudiTestSuite/options"
    ]
    returncode = 130

    def options(self):
        from Configurables import ApplicationMgr, MessageSvc
        from Configurables import GaudiTesting__SignallingAlg as SignallingAlg

        from Gaudi.Configuration import importOptions

        importOptions("Common.opts")
        alg = SignallingAlg(Signal=2)  # SIGINT
        # StopSignalHandler(Signals = ["SIGINT", "SIGXCPU"]) # this is the default

        app = ApplicationMgr(TopAlg=[alg], EvtSel="NONE", EvtMax=5, StopOnSignal=True)  # noqa: F841

        MessageSvc().setDebug.append("EventLoopMgr")

    def filter_out(stdout):
        return "\n".join(
            line
            for line in stdout.splitlines()
            if "EventLoopMgr       SUCCESS Event Number =" not in line
            and " ---> Loop Finished" not in line
        )

    test_check_line = GaudiExeTest.find_reference_block(
        """
        GaudiTesting::S...    INFO 3 events to go
        GaudiTesting::S...    INFO 2 events to go
        GaudiTesting::S...    INFO 1 events to go
        GaudiTesting::S...    INFO Raising signal now
        Gaudi::Utils::S... WARNING Received signal 'SIGINT' (2, Interrupt)
        Gaudi::Utils::S... WARNING Scheduling a stop
        EventLoopMgr       SUCCESS Terminating event processing loop due to a stop scheduled by an incident listener
        ApplicationMgr        INFO Application Manager Stopped successfully
        """,
        preprocessor=filter_out,
    )

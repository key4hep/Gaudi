import re

import pytest

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
from GaudiTesting import GaudiExeTest, platform_matches


@pytest.mark.skipif(
    platform_matches(["asan", "lsan", "ubsan", "tsan"]),
    reason="Unsupported platform",
)
class TestEventTimeout(GaudiExeTest):
    command = ["gaudirun.py"]
    environment = [
        "JOBOPTSEARCHPATH=$ENV_PROJECT_SOURCE_DIR/GaudiTestSuite/options:$ENV_PROJECT_SOURCE_DIR/Gaudi/GaudiTestSuite/options"
    ]

    def options(self):
        from Configurables import ApplicationMgr
        from Configurables import Gaudi__EventWatchdogAlg as EventWatchdogAlg
        from Configurables import GaudiTesting__SleepyAlg as SleepyAlg

        from Gaudi.Configuration import importOptions

        importOptions("Common.opts")

        alg = SleepyAlg("Sleepy", SleepTime=11)
        ewa = EventWatchdogAlg(EventTimeout=2)

        app = ApplicationMgr(TopAlg=[ewa, alg], EvtSel="NONE", EvtMax=2)  # noqa: F841

    def filter_out(stdout):
        stdout = re.sub(r"size = [0-9]+(\.[0-9]*)? MB", "size = # MB", stdout)
        stdout = re.sub(r"(event .* took \d+)\.\d+s", r"\1.###s", stdout)
        stdout = "\n".join(
            l
            for l in stdout.splitlines()
            if "EventLoopMgr      SUCCESS Event Number =" not in l
            and "EventLoopMgr         INFO ---> Loop Finished" not in l
        )
        return stdout

    test_check_line = GaudiExeTest.find_reference_block(
        """
        ApplicationMgr       INFO Application Manager Started successfully
        Sleepy               INFO Executing event 1
        Sleepy               INFO Sleeping for 11 seconds
        EventWatchdog     WARNING More than 2s since the beginning of the event (s: 0  e: 0)
        EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
        EventWatchdog     WARNING Another 2s passed since last timeout (s: 0  e: 0)
        EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
        EventWatchdog     WARNING Another 2s passed since last timeout (s: 0  e: 0)
        EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
        EventWatchdog     WARNING Another 2s passed since last timeout (s: 0  e: 0)
        EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
        EventWatchdog     WARNING Another 2s passed since last timeout (s: 0  e: 0)
        EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
        Sleepy               INFO Back from sleep
        EventWatchdog        INFO An event (s: 0  e: 0) took 11.###s
        Sleepy               INFO Executing event 2
        Sleepy               INFO Sleeping for 11 seconds
        EventWatchdog     WARNING More than 2s since the beginning of the event (s: 0  e: 1)
        EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
        EventWatchdog     WARNING Another 2s passed since last timeout (s: 0  e: 1)
        EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
        EventWatchdog     WARNING Another 2s passed since last timeout (s: 0  e: 1)
        EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
        EventWatchdog     WARNING Another 2s passed since last timeout (s: 0  e: 1)
        EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
        EventWatchdog     WARNING Another 2s passed since last timeout (s: 0  e: 1)
        EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
        Sleepy               INFO Back from sleep
        EventWatchdog        INFO An event (s: 0  e: 1) took 11.###s
        ApplicationMgr       INFO Application Manager Stopped successfully
        """,
        preprocessor=filter_out,
    )

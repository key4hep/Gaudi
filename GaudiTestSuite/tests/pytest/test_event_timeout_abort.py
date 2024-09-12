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
import os
import platform
import re

import pytest
from GaudiTesting import GaudiExeTest, platform_matches


@pytest.mark.skipif(
    "CI" in os.environ or platform_matches(["asan", "lsan", "ubsan", "tsan"]),
    reason="Unsupported platform",
)
class TestEventTimeoutAbort(GaudiExeTest):
    command = ["gaudirun.py"]
    environment = [
        "JOBOPTSEARCHPATH=$ENV_PROJECT_SOURCE_DIR/GaudiTestSuite/options:$ENV_PROJECT_SOURCE_DIR/Gaudi/GaudiTestSuite/options"
    ]
    returncode = -3

    def options(self):
        from Configurables import Gaudi__EventWatchdogAlg as EventWatchdogAlg
        from Configurables import GaudiTesting__SleepyAlg as SleepyAlg

        from Gaudi.Configuration import ApplicationMgr, importOptions

        importOptions("Common.opts")

        alg = SleepyAlg("Sleepy", SleepTime=30)
        ewa = EventWatchdogAlg(EventTimeout=5, AbortOnTimeout=True, StackTrace=True)

        app = ApplicationMgr(TopAlg=[ewa, alg], EvtSel="NONE", EvtMax=2)  # noqa: F841

    def filter_out(stdout):
        return re.subn(r"size = [0-9]+(\.[0-9]*)? MB", "size = # MB", stdout)[0]

    test_stdout = GaudiExeTest.find_reference_block(
        """
        EventWatchdog       FATAL too much time on a single event (s: 0  e: 0): aborting process
        """,
        preprocessor=filter_out,
    )

    def test_stderr(self, stderr):
        expected = b"=== Stalled event: current stack trace (s: 0  e: 0) ==="
        assert expected in stderr

    def test_stack_trace(self, stderr):
        if platform.processor() != "aarch64":
            assert (
                b"in GaudiTesting::SleepyAlg::execute" in stderr
            ), "Invalid stack trace"

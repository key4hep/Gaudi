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
import pytest
from GaudiTesting import GaudiExeTest, platform_matches


@pytest.mark.skipif(
    platform_matches(["asan", "lsan", "ubsan", "tsan"]),
    reason="Unsupported platform",
)
class TestSIGSEGV(GaudiExeTest):
    command = ["gaudirun.py"]
    returncode = 139

    @classmethod
    def update_env(cls, env):
        # Work-around for bug #98693
        #  https://savannah.cern.ch/bugs/?98693
        env["DISPLAY"] = ":0.0"

    def options(self):
        import signal

        from Configurables import GaudiTesting__SignallingAlg

        from Gaudi.Configuration import ApplicationMgr

        app = ApplicationMgr(  # noqa: F841
            TopAlg=[GaudiTesting__SignallingAlg(EventCount=3, Signal=signal.SIGSEGV)],
            EvtSel="NONE",
            EvtMax=5,
        )

    def test_stderr(self):
        # ignore stderr
        pass

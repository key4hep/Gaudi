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
from GaudiTesting.preprocessors import RegexpReplacer, normalizeTestSuite


@pytest.mark.skipif(
    platform_matches(["i686", "asan", "lsan", "ubsan", "tsan"]),
    reason="Unsupported platform",
)
class TestCpuProfiler(GaudiExeTest):
    command = ["gaudirun.py", "--preload=libprofiler.so", "-v"]
    reference = "../refs/google_auditors/cpu_profiler.yaml"
    preprocessor = normalizeTestSuite + RegexpReplacer(
        r"PROFILE: interrupts/evictions/bytes = \d+/\d+/\d+",
        "PROFILE: interrupts/evictions/bytes = X/Y/Z",
    )

    @classmethod
    def update_env(cls, env):
        cls.unset_vars(env, ["GAUDIAPPNAME", "GAUDIAPPVERSION"])

    def options(self):
        from Configurables import (
            GaudiTestSuiteCommonConf,
            Google__CPUProfiler,
            HelloWorld,
        )

        from Gaudi.Configuration import INFO, ApplicationMgr, AuditorSvc

        GaudiTestSuiteCommonConf()
        AuditorSvc().Auditors.append(Google__CPUProfiler(OutputLevel=INFO))
        ApplicationMgr(
            EvtMax=10, EvtSel="NONE", TopAlg=[HelloWorld()], AuditAlgorithms=True
        )

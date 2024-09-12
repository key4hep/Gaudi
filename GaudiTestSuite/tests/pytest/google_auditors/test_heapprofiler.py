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
    platform_matches(["i686", "asan", "lsan", "ubsan", "tsan"]),
    reason="Unsupported platform",
)
class TestHeapProfiler(GaudiExeTest):
    command = ["gaudirun.py", "-T", "-v"]
    reference = "../refs/google_auditors/heap_profiler.yaml"

    @classmethod
    def update_env(cls, env):
        cls.unset_vars(env, ["GAUDIAPPNAME", "GAUDIAPPVERSION"])

    def options(self):
        from Configurables import GaudiTestSuiteCommonConf, HelloWorld
        from Configurables import Google__HeapProfiler as GoogleAuditor

        from Gaudi.Configuration import INFO, ApplicationMgr, AuditorSvc

        GaudiTestSuiteCommonConf()
        AuditorSvc().Auditors.append(GoogleAuditor(OutputLevel=INFO))
        ApplicationMgr(
            EvtMax=10, EvtSel="NONE", TopAlg=[HelloWorld()], AuditAlgorithms=True
        )

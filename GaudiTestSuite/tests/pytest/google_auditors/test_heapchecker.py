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

import pytest
from GaudiTesting import GaudiExeTest, platform_matches
from GaudiTesting.preprocessors import LineSkipper, RegexpReplacer, normalizeTestSuite


@pytest.mark.skipif(
    "CI" in os.environ
    or platform_matches(["i686", "asan", "lsan", "ubsan", "tsan", "^arm", "^aarch"]),
    reason="Unsupported platform",
)
# FIXME We get a segfault in tcmalloc when we use gperftools 2.15 from LCG 106
@pytest.mark.skipif(
    os.environ.get("LCG_VERSION") == "106",
    reason="Unsupported LCG version (gperftools 2.15)",
)
class TestHeapChecker(GaudiExeTest):
    command = ["gaudirun.py", "-T", "-v"]
    reference = "../refs/google_auditors/heap_checker.yaml"
    preprocessor = (
        normalizeTestSuite
        + LineSkipper(
            [
                "Turning perftools heap leak checking off",
                "Perftools heap leak checker is active",
                "Have memory regions w/o callers",
            ]
        )
        + RegexpReplacer(
            r"found \d+ reachable heap objects of \d+ bytes",
            "found XXX reachable heap objects of YYY bytes",
        )
    )

    environment = ["HEAPCHECK=local", "GAUDIAPPNAME=", "GAUDIAPPVERSION="]

    def options(self):
        # Options of the test job
        from Configurables import GaudiTestSuiteCommonConf, HelloWorld
        from Configurables import Google__HeapChecker as GoogleAuditor

        from Gaudi.Configuration import INFO, ApplicationMgr, AuditorSvc

        GaudiTestSuiteCommonConf()
        AuditorSvc().Auditors.append(GoogleAuditor(OutputLevel=INFO))
        ApplicationMgr(
            EvtMax=10, EvtSel="NONE", TopAlg=[HelloWorld()], AuditAlgorithms=True
        )

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
from GaudiTesting.preprocessors import RegexpReplacer, normalizeEOL


class TestBacktrace(GaudiExeTest):
    command = ["gaudirun.py", "-v"]
    environment = ["ENABLE_BACKTRACE=1"]
    returncode = 1

    def options(self):
        from Configurables import ApplicationMgr, TestToolAlgFailure, TestToolFailing

        alg = TestToolAlgFailure(
            Tools=[TestToolFailing("Tool")], IgnoreFailure=False, ThrowException=True
        )

        ApplicationMgr(TopAlg=[alg], EvtSel="NONE", EvtMax=1)

    filter_out = normalizeEOL + RegexpReplacer(
        r"#\d+ +0(x[0-9a-fA-F]{4,16})? +.* +\[.*\]",
        r"#N  0xffffffff signature()  [/path/to/libLibrary.so]",
    )
    test_check_line = GaudiExeTest.find_reference_block(
        """
        TestToolAlgFailure   ERROR Exception stack trace
        #N  0xffffffff signature()  [/path/to/libLibrary.so]
        #N  0xffffffff signature()  [/path/to/libLibrary.so]
        #N  0xffffffff signature()  [/path/to/libLibrary.so]
        """,
        preprocessor=filter_out,
    )

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
from GaudiTesting import GaudiExeTest


class TestBug41136(GaudiExeTest):
    command = ["gaudirun.py", "-v"]
    returncode = 1

    def options(self):
        from Configurables import ApplicationMgr, TestToolAlgFailure, TestToolFailing

        tta1 = TestToolAlgFailure(
            "TestAlg1", Tools=[TestToolFailing("Tool")], IgnoreFailure=True
        )
        tta2 = TestToolAlgFailure(
            "TestAlg2", Tools=[TestToolFailing("Tool")], IgnoreFailure=False
        )

        ApplicationMgr(TopAlg=[tta1, tta2], EvtSel="NONE", EvtMax=1)

    def test_count_error_lines(self, stdout):
        GaudiExeTest.count_error_lines({"ERROR": 8}, stdout.decode())

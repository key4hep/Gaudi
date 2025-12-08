#####################################################################################
# (c) Copyright 2025 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiTesting import GaudiExeTest


class Test(GaudiExeTest):
    command = ["gaudirun.py"]

    def options(self):
        from Configurables import TestToolAlg

        from Gaudi.Configuration import ApplicationMgr, ToolSvc

        ToolSvc().CheckedNamedToolsConfigured = True
        ApplicationMgr(
            EvtMax=1,
            EvtSel="NONE",
            HistogramPersistency="NONE",
            TopAlg=[TestToolAlg(Tools=["TestTool/ToolA"])],
        )

    test_check_line = GaudiExeTest.find_reference_block(
        """
        ToolSvc           WARNING TestTool/ToolSvc.ToolA : Explicitly named tools should be configured! (assigned name=ToolA, default is TestTool)
        """
    )

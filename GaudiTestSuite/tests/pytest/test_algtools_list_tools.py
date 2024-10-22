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


class TestAlgToolsListTools(GaudiExeTest):
    command = ["gaudirun.py", "../../options/AlgTools.py"]

    def options(self):
        from Configurables import ApplicationMgr, GaudiTesting__ListTools

        ApplicationMgr(EvtMax=1).TopAlg.append(GaudiTesting__ListTools("ListTools"))

    def test_tool_list(self, stdout):
        import re

        tools = set(
            line.strip().split()[-1]
            for line in stdout.decode("utf-8").splitlines()
            if re.match(r"^ListTools +INFO   \S+", line)
        )

        expected = set(
            [
                "ToolSvc.MyTool",
                "MyAlg.MyTool",
                "MyAlg.ToolWithName",
                "MyAlg.LegacyToolHandle",
                "MyAlg.MyTool",
                "MyAlg.ToolWithName",
                "MyAlg.PrivToolHandle",
                "MyAlg.WrongIfaceTool",
                "ToolSvc.AnotherConstGenericToolHandle",
                "ToolSvc.AnotherInstanceOfMyTool",
                "ToolSvc.TestPubToolHandle",
                "ToolSvc.GenericToolHandle",
                "ToolSvc.ConstGenericToolHandle",
                "ToolSvc.ToolA",
                "ToolSvc.ToolB",
                "ToolSvc.PubToolHandle",
            ]
        )

        missing = expected - tools
        extra = tools - expected

        assert not missing, f"Missing tools: {sorted(missing)}"
        assert not extra, f"Extra tools: {sorted(extra)}"

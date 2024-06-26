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
import re

from GaudiTesting import GaudiExeTest


class Test(GaudiExeTest):
    command = ["gaudirun.py", "-v"]

    def options(self):
        from Configurables import AuditorTestAlg
        from Configurables import GaudiTestSuite__LoggingAuditor as LoggingAuditor

        from Gaudi.Configuration import (
            ApplicationMgr,
            AuditorSvc,
            MessageSvc,
            TimingAuditor,
            ToolSvc,
        )

        AuditorSvc().Auditors += [
            TimingAuditor("TIMER"),
            LoggingAuditor("LoggingAuditor"),
        ]

        app = ApplicationMgr(TopAlg=[AuditorTestAlg()], EvtSel="NONE", EvtMax=5)

        app.ExtSvc += [ToolSvc(), AuditorSvc()]
        app.AuditAlgorithms = True

        MessageSvc().setDebug.append("EventLoopMgr")

    def test_stdout(self, stdout: bytes):
        assert re.search(
            b"TIMER.TIMER *INFO AuditorTestAlg:loop", stdout
        ), "missing timing report"

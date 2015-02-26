# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="gaudirun.py"
        self.args=["-v"]
        self.options="""
from Gaudi.Configuration import *

from Configurables import AuditorTestAlg, GaudiExamples__LoggingAuditor as LoggingAuditor

importOptions("Common.opts")

AuditorSvc().Auditors += [ TimingAuditor("TIMER"),
                           LoggingAuditor("LoggingAuditor") ]

app = ApplicationMgr(TopAlg = [AuditorTestAlg()],
                     EvtSel = "NONE", EvtMax = 5)

app.ExtSvc += [ ToolSvc(), AuditorSvc() ]
app.AuditAlgorithms = True

MessageSvc().setDebug.append("EventLoopMgr") """

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        import re
        if not re.search("TIMER.TIMER *INFO AuditorTestAlg:loop",stdout):
            causes.append("missing timing reoprt")
            result["GaudiTest.expected"] = result.Quote("timing report for timer 'AuditorTestAlg:loop'")
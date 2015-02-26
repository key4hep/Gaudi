# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="gaudirun.py"
        self.options="""
from Gaudi.Configuration import *
from Configurables import GaudiExamples__CounterSvcAlg

counter2 = GaudiExamples__CounterSvcAlg("Counter2",CounterBaseName = "Test2")

seq = Sequencer(Members = [ GaudiExamples__CounterSvcAlg.getType(),
                            counter2,
                            "GaudiExamples::StatSvcAlg:invert" ])

ApplicationMgr(TopAlg = [seq], EvtSel = "NONE", EvtMax = 1)"""

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        expected = "Sequencer            INFO Member list: GaudiExamples::CounterSvcAlg, GaudiExamples::CounterSvcAlg/Counter2, GaudiExamples::StatSvcAlg:invert"
        if not expected in stdout:
            causes.append("missing signature")
            result["GaudiTest.expected_line"] = result.Quote(expected)
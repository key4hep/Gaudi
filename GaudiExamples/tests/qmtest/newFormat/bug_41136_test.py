# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="gaudirun.py"
        self.args=["-v"]
        self.exit_code=1
        self.options="""
from Gaudi.Configuration import *
from Configurables import TestToolFailing, TestToolAlgFailure

tta1 = TestToolAlgFailure("TestAlg1", Tools = [ TestToolFailing("Tool") ], IgnoreFailure = True)
tta2 = TestToolAlgFailure("TestAlg2", Tools = [ TestToolFailing("Tool") ], IgnoreFailure = False)

ApplicationMgr(TopAlg = [ tta1, tta2 ],
               EvtSel = "NONE",
               EvtMax = 1)
"""

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        self.countErrorLines({"FATAL":2,
                         "ERROR":7,
                         })
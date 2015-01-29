# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="gaudirun.py"
        self.signal=6
        self.options="""
from Gaudi.Configuration import *

from Configurables import GaudiTesting__SleepyAlg as SleepyAlg
from Configurables import StalledEventMonitor

importOptions("Common.opts")

alg = SleepyAlg("Sleepy", SleepTime = 10)
sem = StalledEventMonitor(EventTimeout = 5,
                          MaxTimeoutCount = 1,
                          StackTrace = True)

app = ApplicationMgr(TopAlg = [alg],
                     EvtSel = "NONE", EvtMax = 2,
                     StalledEventMonitoring = True)
"""

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        import re
        stdout = re.subn(r"size = [0-9]+(\.[0-9]*)? MB", "size = # MB", stdout)[0]

        self.findReferenceBlock("EventWatchdog       FATAL too much time on a single event: aborting process")

        self.findReferenceBlock("=== Stalled event: current stack trace ===", stdout=stderr, id='stderr')

        if not 'in GaudiTesting::SleepyAlg::execute' in stderr:
            causes.append('invalid stack trace')

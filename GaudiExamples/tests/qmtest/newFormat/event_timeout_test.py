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

from Configurables import GaudiTesting__SleepyAlg as SleepyAlg
from Configurables import StalledEventMonitor

importOptions("Common.opts")

alg = SleepyAlg("Sleepy", SleepTime = 11)
sem = StalledEventMonitor(EventTimeout = 2)

app = ApplicationMgr(TopAlg = [alg],
                     EvtSel = "NONE", EvtMax = 2,
                     StalledEventMonitoring = True)
"""

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        import re
        stdout = re.subn(r"size = [0-9]+(\.[0-9]*)? MB", "size = # MB", stdout)[0]

        self.findReferenceBlock("""
ApplicationMgr       INFO Application Manager Started successfully
Sleepy               INFO Executing event 1
Sleepy               INFO Sleeping for 11 seconds
EventWatchdog     WARNING More than 2s since the last BeginEvent
EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
EventWatchdog     WARNING Other 2s passed
EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
EventWatchdog     WARNING Other 2s passed
EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
EventWatchdog     WARNING Other 2s passed
EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
EventWatchdog     WARNING Other 2s passed
EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
Sleepy               INFO Back from sleep
EventWatchdog        INFO Starting a new event after ~10s
Sleepy               INFO Executing event 2
Sleepy               INFO Sleeping for 11 seconds
EventWatchdog     WARNING More than 2s since the last BeginEvent
EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
EventWatchdog     WARNING Other 2s passed
EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
EventWatchdog     WARNING Other 2s passed
EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
EventWatchdog     WARNING Other 2s passed
EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
EventWatchdog     WARNING Other 2s passed
EventWatchdog        INFO Current memory usage is virtual size = # MB, resident set size = # MB
Sleepy               INFO Back from sleep
EventWatchdog        INFO The last event took ~10s
ApplicationMgr       INFO Application Manager Stopped successfully
""", stdout = stdout)
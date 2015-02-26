# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="gaudirun.py"
        self.exit_code=130
        self.options="""
from Gaudi.Configuration import *

from Configurables import GaudiTesting__SignallingAlg as SignallingAlg
from Configurables import Gaudi__Utils__StopSignalHandler as StopSignalHandler

importOptions("Common.opts")
alg = SignallingAlg(Signal = 2) # SIGINT
#StopSignalHandler(Signals = ["SIGINT", "SIGXCPU"]) # this is the default

app = ApplicationMgr(TopAlg = [alg],
                     EvtSel = "NONE", EvtMax = 5,
                     StopOnSignal = True)

MessageSvc().setDebug.append("EventLoopMgr")"""

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        self.findReferenceBlock("""
GaudiTesting::S...   INFO 3 events to go
GaudiTesting::S...   INFO 2 events to go
GaudiTesting::S...   INFO 1 events to go
GaudiTesting::S...   INFO Raising signal now
Gaudi::Utils::S...WARNING Received signal 'SIGINT' (2, Interrupt)
Gaudi::Utils::S...WARNING Scheduling a stop
EventLoopMgr      SUCCESS Terminating event processing loop due to a stop scheduled by an incident listener
ApplicationMgr       INFO Application Manager Stopped successfully
""")
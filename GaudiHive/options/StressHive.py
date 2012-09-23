#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
# ============================================================================
from Configurables import GaudiExamplesCommonConf, CPUCruncher,HiveEventLoopMgr_v2
#GaudiExamplesCommonConf()
# ============================================================================

number_of_algos = 20
number_of_threads = 10
mean_time = 1

# ============================================================================
# Create number_of_algos independent algos and run them
import random
auto_crunchers=[]
for i in xrange(number_of_algos):
  auto_cruncher = CPUCruncher("cruncher_%s"%i,
                              avgRuntime=random.uniform(mean_time+.1,mean_time+.5),
                              Inputs=[],
                              Outputs=["coll_%s"%i])
  auto_crunchers.append(auto_cruncher)

# ============================================================================

# Setup the Event Loop Manager
evtloop = HiveEventLoopMgr_v2()
evtloop.MaxEventsParallel = number_of_threads
evtloop.NumThreads = number_of_threads
evtloop.CloneAlgorithms = True
evtloop.DumpQueues = False

# And the Application Manager

app = ApplicationMgr()
app.TopAlg = auto_crunchers
app.EvtSel = "NONE" # do not use any event input
app.EvtMax = 10
app.EventLoop = evtloop;
app.MessageSvcType = "TBBMessageSvc";



# ============================================================================
# The END
# ============================================================================

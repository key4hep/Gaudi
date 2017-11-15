#!/usr/bin/env gaudirun.py

"""
Find and attribute unmet data inputs as outputs to a Data Loader algorithm.
"""

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, AvalancheSchedulerSvc, CPUCruncher

# metaconfig
evtslots = 1
evtMax = 3
algosInFlight = 1


whiteboard = HiveWhiteBoard("EventDataSvc",
                            EventSlots=evtslots,
                            OutputLevel=INFO)

slimeventloopmgr = HiveSlimEventLoopMgr(SchedulerName="AvalancheSchedulerSvc")

AvalancheSchedulerSvc(ThreadPoolSize=algosInFlight,
                      CheckDependencies=True,
                      DataLoaderAlg="AlgA")

# Assemble the data flow graph
a1 = CPUCruncher("AlgA", Loader=True, OutputLevel=VERBOSE)

a2 = CPUCruncher("AlgB", OutputLevel=VERBOSE)
a2.inpKeys = ['/Event/A1']

a3 = CPUCruncher("AlgC", OutputLevel=VERBOSE)
a3.inpKeys = ['/Event/A2']

for a in [a1, a2, a3]:
    a.shortCalib = True
    a.avgRuntime = .01


ApplicationMgr(EvtMax=evtMax,
               EvtSel='NONE',
               ExtSvc=[whiteboard],
               EventLoop=slimeventloopmgr,
               TopAlg=[a1, a2, a3],
               MessageSvcType="InertMessageSvc",
               OutputLevel=DEBUG)

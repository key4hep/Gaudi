#!/usr/bin/env gaudirun.py

# Just hammer the logger with a lot of messages

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, ForwardSchedulerSvc, CPUCruncher

verbosity = VERBOSE

InertMessageSvc(OutputLevel=verbosity)
TBBMessageSvc(OutputLevel=verbosity)

# metaconfig
evtslots = 10
evtMax = 50
cardinality = 20
algosInFlight = 20

whiteboard = HiveWhiteBoard("EventDataSvc",
                            EventSlots=evtslots)

slimeventloopmgr = HiveSlimEventLoopMgr(OutputLevel=verbosity)

scheduler = ForwardSchedulerSvc(MaxEventsInFlight=evtslots,
                                MaxAlgosInFlight=algosInFlight,
                                OutputLevel=verbosity,
                                AlgosDependencies=[[], []])

a1 = CPUCruncher("A1",
                 DataOutputs=['/Event/a1'],
                 varRuntime=.1,
                 avgRuntime=.5)
a2 = CPUCruncher("A2",
                 DataInputs=[],
                 DataOutputs=['/Event/a2'])

for algo in [a1, a2]:
    algo.Cardinality = cardinality
    algo.shortCalib = True
    algo.OutputLevel = verbosity

ApplicationMgr(EvtMax=evtMax,
               EvtSel='NONE',
               ExtSvc=[whiteboard],
               EventLoop=slimeventloopmgr,
               TopAlg=[a1, a2],
               MessageSvcType="InertMessageSvc")

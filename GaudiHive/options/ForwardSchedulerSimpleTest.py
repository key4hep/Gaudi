#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, ForwardSchedulerSvc, CPUCruncher

TBBMessageSvc(OutputLevel=WARNING)

# metaconfig
evtslots = 13
evtMax = 50
cardinality=10
algosInFlight=10

whiteboard   = HiveWhiteBoard("EventDataSvc",
                              EventSlots = evtslots)

slimeventloopmgr = HiveSlimEventLoopMgr(OutputLevel=INFO)

scheduler = ForwardSchedulerSvc(MaxEventsInFlight = evtslots,
                                MaxAlgosInFlight = algosInFlight,
                                OutputLevel=WARNING,
                                AlgosDependencies = [[],['a1'],['a1'],['a2','a3']])

a1 = CPUCruncher("A1", 
                 Outputs = ['/Event/a1'],
                 shortCalib=True,
                 varRuntime=.1, 
                 avgRuntime=.5 )
a2 = CPUCruncher("A2", 
                 shortCalib=True,
                 Inputs = ['/Event/a1'],
                 Outputs = ['/Event/a2'])
a3 = CPUCruncher("A3", 
                 shortCalib=True,
                 Inputs = ['/Event/a1'],
                 Outputs = ['/Event/a3'])
a4 = CPUCruncher("A4", 
                 shortCalib=True,
                 Inputs = ['/Event/a2','/Event/a3'],
                 Outputs = ['/Event/a4'])

for algo in [a1,a2,a3,a4]:
  algo.Cardinality = cardinality
  algo.OutputLevel=WARNING

ApplicationMgr( EvtMax = evtMax,
                EvtSel = 'NONE',
                ExtSvc =[whiteboard],
                EventLoop = slimeventloopmgr,
                TopAlg = [a1,a2,a3,a4],
                MessageSvcType="TBBMessageSvc")

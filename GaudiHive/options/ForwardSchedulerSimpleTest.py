#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, ForwardSchedulerSvc, CPUCruncher,AlgResourcePool

InertMessageSvc(OutputLevel=INFO)

# metaconfig
evtslots = 23
evtMax = 50
cardinality=10
algosInFlight=10

whiteboard   = HiveWhiteBoard("EventDataSvc",
                              EventSlots = evtslots)

slimeventloopmgr = HiveSlimEventLoopMgr(OutputLevel=INFO)

scheduler = ForwardSchedulerSvc(MaxEventsInFlight = evtslots,
                                MaxAlgosInFlight = algosInFlight,
                                OutputLevel=WARNING)

AlgResourcePool(OutputLevel=DEBUG)
                                
a1 = CPUCruncher("A1", 
                 Outputs = ['/Event/a1'])
a2 = CPUCruncher("A2", 
                 Inputs = ['a1'],
                 Outputs = ['/Event/a2'])
a3 = CPUCruncher("A3", 
                 Inputs = ['a1'],
                 Outputs = ['/Event/a3'])
a4 = CPUCruncher("A4", 
                 Inputs = ['a2','a3'],
                 Outputs = ['/Event/a4'])

for algo in [a1,a2,a3,a4]:
  algo.shortCalib=True
  algo.Cardinality = cardinality
  algo.OutputLevel=WARNING
  algo.varRuntime=.3
  algo.avgRuntime=.5  

ApplicationMgr( EvtMax = evtMax,
                EvtSel = 'NONE',
                ExtSvc =[whiteboard],
                EventLoop = slimeventloopmgr,
                TopAlg = [a1,a2,a3,a4],
                MessageSvcType="InertMessageSvc")

#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, ForwardSchedulerSvc, CPUCruncher

InertMessageSvc(OutputLevel=WARNING)

# metaconfig
evtslots = 10
evtMax = 15
cardinality=20
algosInFlight=20

whiteboard   = HiveWhiteBoard("EventDataSvc",
                              EventSlots = evtslots)

slimeventloopmgr = HiveSlimEventLoopMgr(OutputLevel=INFO)

scheduler = ForwardSchedulerSvc(MaxAlgosInFlight = algosInFlight,
                                OutputLevel=WARNING)

a1 = CPUCruncher("A1", 
                 Outputs = ['/Event/a1'],
                 shortCalib=True,
                 varRuntime=.1, 
                 avgRuntime=.5 )
a2 = CPUCruncher("A2", 
                 shortCalib=True,
                 Inputs = ['a1'],
                 Outputs = ['/Event/a2'])
a3 = CPUCruncher("A3", 
                 shortCalib=True,
                 Inputs = ['a1'],
                 Outputs = ['/Event/a3UPS!']) #<=== MISTAKE!
a4 = CPUCruncher("A4", 
                 shortCalib=True,
                 Inputs = ['a2','a3'],
                 Outputs = ['/Event/a4'])

for algo in [a1,a2,a3,a4]:
  algo.Cardinality = cardinality
  algo.OutputLevel=WARNING

ApplicationMgr( EvtMax = evtMax,
                EvtSel = 'NONE',
                ExtSvc =[whiteboard],
                EventLoop = slimeventloopmgr,
                TopAlg = [a1,a2,a3,a4],
                MessageSvcType="InertMessageSvc")

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
                 shortCalib=True,
                 varRuntime=.1, 
                 avgRuntime=.5 )
a1.OutputDataObjects.output_0.address = '/Event/a1'

a2 = CPUCruncher("A2", 
                 shortCalib=True)
a2.InputDataObjects.input_0.address = 'a1'
a2.OutputDataObjects.output_0.address = '/Event/a2'

a3 = CPUCruncher("A3", 
                 shortCalib=True)
a3.InputDataObjects.input_0.address = 'a1'
a3.OutputDataObjects.output_0.address = '/Event/a3UPS!' #<=== MISTAKE!

a4 = CPUCruncher("A4", 
                 shortCalib=True)
a4.InputDataObjects.input_0.address = 'a2'
a4.InputDataObjects.input_1.address = 'a3'
a4.OutputDataObjects.output_0.address = '/Event/a4'

for algo in [a1,a2,a3,a4]:
  algo.Cardinality = cardinality
  algo.OutputLevel=WARNING

ApplicationMgr( EvtMax = evtMax,
                EvtSel = 'NONE',
                ExtSvc =[whiteboard],
                EventLoop = slimeventloopmgr,
                TopAlg = [a1,a2,a3,a4],
                MessageSvcType="InertMessageSvc")

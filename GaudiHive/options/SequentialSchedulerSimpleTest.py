#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, SequentialSchedulerSvc, CPUCruncher,AlgResourcePool

InertMessageSvc(OutputLevel=INFO)

# metaconfig
evtMax = 10

scheduler = SequentialSchedulerSvc(OutputLevel=DEBUG)

slimeventloopmgr = HiveSlimEventLoopMgr(SchedulerName="SequentialSchedulerSvc",
                                        OutputLevel=DEBUG)

whiteboard   = HiveWhiteBoard("EventDataSvc")                                        
                                        

algResPool=AlgResourcePool(OutputLevel=DEBUG)
                                
a1 = CPUCruncher("A1", 
                 shortCalib=True,
                 varRuntime=.1, 
                 avgRuntime=.5 )
a1.OutputDataObjects.output_0.address = 'a1'

a2 = CPUCruncher("A2", 
                 shortCalib=True)
a2.InputDataObjects.input_0.address = 'a1'
a2.OutputDataObjects.output_0.address = 'a2'

a3 = CPUCruncher("A3", 
                 shortCalib=True)
a3.InputDataObjects.input_0.address = 'a1'
a3.OutputDataObjects.output_0.address = 'a3'

a4 = CPUCruncher("A4", 
                 shortCalib=True)
a4.InputDataObjects.input_0.address = 'a2'
a4.InputDataObjects.input_1.address = 'a3'
a4.OutputDataObjects.output_0.address = 'a4'


for algo in [a1,a2,a3,a4]:
  algo.OutputLevel=INFO

ApplicationMgr( EvtMax = evtMax,
                EvtSel = 'NONE',
                ExtSvc =[whiteboard,algResPool],
                EventLoop = slimeventloopmgr,
                TopAlg = [a1,a2,a3,a4],
                MessageSvcType="InertMessageSvc")

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
a1.Outputs.output_0.Path = 'a1'

a2 = CPUCruncher("A2", 
                 shortCalib=True)
a2.Inputs.input_0.Path = 'a1'
a2.Outputs.output_0.Path = 'a2'

a3 = CPUCruncher("A3", 
                 shortCalib=True)
a3.Inputs.input_0.Path = 'a1'
a3.Outputs.output_0.Path = 'a3'

a4 = CPUCruncher("A4", 
                 shortCalib=True)
a4.Inputs.input_0.Path = 'a2'
a4.Inputs.input_1.Path = 'a3'
a4.Outputs.output_0.Path = 'a4'


for algo in [a1,a2,a3,a4]:
  algo.OutputLevel=INFO

ApplicationMgr( EvtMax = evtMax,
                EvtSel = 'NONE',
                ExtSvc =[whiteboard,algResPool],
                EventLoop = slimeventloopmgr,
                TopAlg = [a1,a2,a3,a4],
                MessageSvcType="InertMessageSvc")

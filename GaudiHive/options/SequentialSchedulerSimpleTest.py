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
a1.DataOutputs.output_0.Path = '/Event/a1'

a2 = CPUCruncher("A2", 
                 shortCalib=True)
a2.DataInputs.input_0.Path = '/Event/a1'
a2.DataOutputs.output_0.Path = '/Event/a2'

a3 = CPUCruncher("A3", 
                 shortCalib=True)
a3.DataInputs.input_0.Path = '/Event/a1'
a3.DataOutputs.output_0.Path = '/Event/a3'

a4 = CPUCruncher("A4", 
                 shortCalib=True)
a4.DataInputs.input_0.Path = '/Event/a2'
a4.DataInputs.input_1.Path = '/Event/a3'
a4.DataOutputs.output_0.Path = '/Event/a4'


for algo in [a1,a2,a3,a4]:
  algo.OutputLevel=INFO

ApplicationMgr( EvtMax = evtMax,
                EvtSel = 'NONE',
                ExtSvc =[whiteboard,algResPool],
                EventLoop = slimeventloopmgr,
                TopAlg = [a1,a2,a3,a4],
                MessageSvcType="InertMessageSvc")

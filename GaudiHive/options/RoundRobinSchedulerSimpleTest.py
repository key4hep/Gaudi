#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, RoundRobinSchedulerSvc, CPUCruncher,AlgResourcePool

InertMessageSvc(OutputLevel=INFO)

# metaconfig
evtMax = 10

scheduler = RoundRobinSchedulerSvc(OutputLevel=DEBUG)

slimeventloopmgr = HiveSlimEventLoopMgr(SchedulerName="RoundRobinSchedulerSvc",
                                        OutputLevel=DEBUG)

whiteboard   = HiveWhiteBoard("EventDataSvc")                                        
                                        

algResPool=AlgResourcePool(OutputLevel=DEBUG)
                                
a1 = CPUCruncher("A1", 
                 DataOutputs = ['/Event/a1'],
                 shortCalib=True,
                 varRuntime=.1, 
                 avgRuntime=.5 )
a2 = CPUCruncher("A2", 
                 shortCalib=True,
                 DataInputs = ['/Event/a1'],
                 DataOutputs = ['/Event/a2'])
a3 = CPUCruncher("A3", 
                 shortCalib=True,
                 DataInputs = ['/Event/a1'],
                 DataOutputs = ['/Event/a3'])
a4 = CPUCruncher("A4", 
                 shortCalib=True,
                 DataInputs = ['/Event/a2','/Event/a3'],
                 DataOutputs = ['/Event/a4'])

for algo in [a1,a2,a3,a4]:
  algo.OutputLevel=INFO

ApplicationMgr( EvtMax = evtMax,
                EvtSel = 'NONE',
                ExtSvc =[whiteboard,algResPool],
                EventLoop = slimeventloopmgr,
                TopAlg = [a1,a2,a3,a4],
                MessageSvcType="InertMessageSvc")

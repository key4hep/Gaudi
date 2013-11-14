#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, ParallelSequentialSchedulerSvc, CPUCruncher,AlgResourcePool

InertMessageSvc(OutputLevel=DEBUG)

# metaconfig
evtMax = 10
evtInFlight = 10

scheduler = ParallelSequentialSchedulerSvc(OutputLevel=DEBUG)

slimeventloopmgr = HiveSlimEventLoopMgr(SchedulerName="ParallelSequentialSchedulerSvc",
                                        OutputLevel=DEBUG)

whiteboard   = HiveWhiteBoard("EventDataSvc",
			EventSlots = evtInFlight)
                                        

algResPool = AlgResourcePool(OutputLevel=DEBUG)
                                
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
  algo.OutputLevel=INFO
  algo.Cardinality=1

ApplicationMgr( EvtMax = evtMax,
                EvtSel = 'NONE',
                ExtSvc =[whiteboard,algResPool],
                EventLoop = slimeventloopmgr,
                TopAlg = [a1,a2,a3,a4],
                MessageSvcType="InertMessageSvc")

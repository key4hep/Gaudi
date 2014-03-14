#!/usr/bin/env gaudirun.py

'''
Options file to test fix for https://sft.its.cern.ch/jira/browse/CFHEP-114
a problem which is ther when declaring dependencies explicitely in the scheduler
and having more dependencies than algorithms.
'''

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, ForwardSchedulerSvc, CPUCruncher,AlgResourcePool

InertMessageSvc(OutputLevel=INFO)

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
                                AlgosDependencies = [[],['/Event/a1'],['/Event/a1'],['/Event/a2','/Event/a3','/Event/a4']])

AlgResourcePool(OutputLevel=DEBUG)

a1 = CPUCruncher("A1",
                 shortCalib=True,
                 varRuntime=.01,
                 avgRuntime=.1 )
a1.OutputDataObjects.output_0.address = '/Event/a1'

a2 = CPUCruncher("A2",
                 shortCalib=True)
a2.OutputDataObjects.output_0.address = '/Event/a2'

a3 = CPUCruncher("A3",
                 shortCalib=True)
a3.OutputDataObjects.output_0.address = '/Event/a3'
a3.OutputDataObjects.output_1.address = '/Event/a4'

a4 = CPUCruncher("A4",
                 shortCalib=True)
a4.OutputDataObjects.output_0.address = '/Event/a5'

for algo in [a1,a2,a3,a4]:
  algo.Cardinality = cardinality
  algo.OutputLevel=WARNING

ApplicationMgr( EvtMax = evtMax,
                EvtSel = 'NONE',
                ExtSvc =[whiteboard],
                EventLoop = slimeventloopmgr,
                TopAlg = [a1,a2,a3,a4],
                MessageSvcType="InertMessageSvc")

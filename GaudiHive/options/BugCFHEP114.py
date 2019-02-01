#!/usr/bin/env gaudirun.py
'''
Options file to test fix for https://sft.its.cern.ch/jira/browse/CFHEP-114
a problem which is ther when declaring dependencies explicitely in the scheduler
and having more dependencies than algorithms.
'''

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, AvalancheSchedulerSvc, CPUCruncher, AlgResourcePool

InertMessageSvc(OutputLevel=INFO)

# metaconfig
evtslots = 13
evtMax = 50
cardinality = 10
algosInFlight = 10

whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots)

slimeventloopmgr = HiveSlimEventLoopMgr(OutputLevel=INFO)

scheduler = AvalancheSchedulerSvc(
    ThreadPoolSize=algosInFlight, OutputLevel=WARNING)

AlgResourcePool(OutputLevel=DEBUG)

a1 = CPUCruncher("A1", shortCalib=True, varRuntime=.01, avgRuntime=.1)
a1.outKeys = ['/Event/a1']

a2 = CPUCruncher("A2", shortCalib=True)
a2.outKeys = ['/Event/a2']

a3 = CPUCruncher("A3", shortCalib=True)
a3.outKeys = ['/Event/a3', '/Event/a4']

a4 = CPUCruncher("A4", shortCalib=True)
a4.outKeys = ['/Event/a5']

for algo in [a1, a2, a3, a4]:
    algo.Cardinality = cardinality
    algo.OutputLevel = WARNING

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel='NONE',
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[a1, a2, a3, a4],
    MessageSvcType="InertMessageSvc")

#!/usr/bin/env gaudirun.py
"""
A test modeling a production with subsequent modification of a data object. '/Event/B' experiences a conditional modification,
while '/Event/A' - an unconditional one:
 (C): '/Event/B' <---- | UpdaterAlg3 (C) | ProducerAlg2 (C) |
 (U): '/Event/A' <---- | UpdaterAlg2 (U) | ProducerAlg1 (C) | UpdaterAlg4 (U) | UpdaterAlg1 (C) |
"""

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, AvalancheSchedulerSvc, CPUCruncher, CPUCrunchSvc, PrecedenceSvc

# metaconfig
evtMax = 1
evtslots = 1
algosInFlight = 1

CPUCrunchSvc(shortCalib=True)

PrecedenceSvc(OutputLevel=DEBUG)

whiteboard = HiveWhiteBoard(
    "EventDataSvc", EventSlots=evtslots, OutputLevel=INFO)

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=INFO)

AvalancheSchedulerSvc(ThreadPoolSize=algosInFlight)

producerAlg1 = CPUCruncher(name="ProducerAlg1")
producerAlg1.outKeys = ["/Event/A"]

updaterAlg1 = CPUCruncher(name="UpdaterAlg1")
updaterAlg1.outKeys = ["/Event/A"]

updaterAlg2 = CPUCruncher(name="UpdaterAlg2")
updaterAlg2.outKeys = ["/Event/A"]

producerAlg2 = CPUCruncher(name="ProducerAlg2")
producerAlg2.inpKeys = ["/Event/A"]
producerAlg2.outKeys = ["/Event/B"]

updaterAlg3 = CPUCruncher(name="UpdaterAlg3")
updaterAlg3.outKeys = ["/Event/B"]

updaterAlg4 = CPUCruncher(name="UpdaterAlg4")
updaterAlg4.outKeys = ["/Event/A"]

branch2 = GaudiSequencer(
    "ConditionalBranch", Sequential=True, ShortCircuit=True)
branch2.Members = [producerAlg1, updaterAlg1, producerAlg2, updaterAlg3]

branch = GaudiSequencer("UnConditionalBranch", ShortCircuit=False)
branch.Members = [branch2, updaterAlg2, updaterAlg4]

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel='NONE',
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[branch],
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO)

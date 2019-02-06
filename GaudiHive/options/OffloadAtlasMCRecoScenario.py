#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, AvalancheSchedulerSvc, IOBoundAlgSchedulerSvc, CPUCrunchSvc

# convenience machinery for assembling custom graphs of algorithm precedence rules (w/ CPUCrunchers as algorithms)
from GaudiHive import precedence

# metaconfig
evtslots = 1
evtMax = 10
algosInFlight = 4

InertMessageSvc(OutputLevel=INFO)

whiteboard = HiveWhiteBoard(
    "EventDataSvc", EventSlots=evtslots, OutputLevel=INFO)

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=INFO)

scheduler = AvalancheSchedulerSvc(
    ThreadPoolSize=algosInFlight,
    OutputLevel=INFO,
    PreemptiveIOBoundTasks=True,
    MaxIOBoundAlgosInFlight=50,
    DumpIntraEventDynamics=True)

IOBoundAlgSchedulerSvc(OutputLevel=INFO)

CPUCrunchSvc(shortCalib=True)

#timeValue = precedence.UniformTimeValue(avgRuntime=0.1)
timeValue = precedence.RealTimeValue(
    path="atlas/mcreco/averageTiming.mcreco.TriggerOff.json", defaultTime=0.0)
#ifIObound = precedence.UniformBooleanValue(False)
ifIObound = precedence.RndBiasedBooleanValue(
    pattern={
        True: 17,
        False: 152
    }, seed=1)

sequencer = precedence.CruncherSequence(
    timeValue,
    ifIObound,
    sleepFraction=0.9,
    cfgPath="atlas/mcreco/cf.mcreco.TriggerOff.graphml",
    dfgPath="atlas/mcreco/df.mcreco.TriggerOff.3rdEvent.graphml",
    topSequencer='AthSequencer/AthMasterSeq').get()

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel='NONE',
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[sequencer],
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO)

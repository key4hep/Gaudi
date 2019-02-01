#!/usr/bin/env gaudirun.py
"""
The simplest possible configuration for asynchronous scheduling of single blocking algorithm.
"""

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, AvalancheSchedulerSvc, CPUCruncher

# metaconfig
evtMax = 7
evtslots = 3
algosInFlight = 0
blockingAlgosInFlight = 3

whiteboard = HiveWhiteBoard(
    "EventDataSvc", EventSlots=evtslots, OutputLevel=INFO)

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=INFO)

AvalancheSchedulerSvc(
    ThreadPoolSize=algosInFlight,
    PreemptiveIOBoundTasks=True,
    MaxIOBoundAlgosInFlight=blockingAlgosInFlight,
    OutputLevel=VERBOSE)

blockingAlg = CPUCruncher(
    name="BlockingAlg",
    shortCalib=True,
    avgRuntime=2.,
    Cardinality=3,
    IsIOBound=True,  # tag algorithm as blocking
    SleepFraction=0.7,  # simulate blocking nature
    OutputLevel=DEBUG)

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel='NONE',
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[blockingAlg],
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO)

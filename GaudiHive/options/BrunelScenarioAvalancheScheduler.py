#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, AvalancheSchedulerSvc, AlgResourcePool, CPUCrunchSvc

# convenience machinery for assembling custom graphs of algorithm precedence rules (w/ CPUCrunchers as algorithms)
try:
    from GaudiHive import precedence
except ImportError:
    # of versions of LCG/heptools do not provide the required package networkx
    import sys
    sys.exit(77)  # consider the test skipped

# metaconfig
evtslots = 1
evtMax = 1
cardinality = 1
algosInFlight = 4
algoAvgTime = 0.02

InertMessageSvc(OutputLevel=INFO)

whiteboard = HiveWhiteBoard(
    "EventDataSvc", EventSlots=evtslots, OutputLevel=INFO, ForceLeaves=True)

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=INFO)

scheduler = AvalancheSchedulerSvc(
    ThreadPoolSize=algosInFlight,
    OutputLevel=DEBUG,
    Optimizer="DRE",
    PreemptiveIOBoundTasks=False,
    DumpIntraEventDynamics=False)

AlgResourcePool(OutputLevel=DEBUG)

CPUCrunchSvc(shortCalib=True)

timeValue = precedence.UniformTimeValue(avgRuntime=algoAvgTime)
#timeValue = precedence.RealTimeValue(
#    path="lhcb/reco/timing.Brunel.1kE.json", defaultTime=0.0)
ifIObound = precedence.UniformBooleanValue(False)
# 296 values, biased approximately as 90% to 10% - corresponds to the .GRAPHML scenario used below
# (295 precedence graph algorithms, plus one fake algorithm - FetchFromFile)
#ifIObound = precedence.RndBiasedBoolenValue(pattern = {True: 29, False: 267}, seed=1)

sequencer = precedence.CruncherSequence(
    timeValue,
    ifIObound,
    sleepFraction=0.0,
    cfgPath="lhcb/reco/cf.Brunel.graphml",
    dfgPath="lhcb/reco/df.Brunel.graphml",
    topSequencer='BrunelSequencer').get()

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel='NONE',
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[sequencer],
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO)

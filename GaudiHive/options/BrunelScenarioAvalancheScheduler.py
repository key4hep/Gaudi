#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, AvalancheSchedulerSvc, AlgResourcePool

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

timeValue = precedence.UniformTimeValue(avgRuntime=algoAvgTime)
ifIObound = precedence.UniformBooleanValue(False)
# 278 values, biased approximately as 90% to 10% - corresponds to the .GRAPHML scenario used below
# (276 precedence graph algorithms, plus two fake algorithms - DstWriter and Framework)
#ifIObound = precedence.RndBiasedBoolenValue(pattern = {True: 29, False: 249}, seed=1)

sequencer = precedence.CruncherSequence(
    timeValue,
    ifIObound,
    sleepFraction=0.0,
    cfgPath="lhcb/reco/cf_dependencies.graphml",
    dfgPath="lhcb/reco/data_dependencies.graphml",
    topSequencer='GaudiSequencer/BrunelSequencer').get()

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel='NONE',
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[sequencer],
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO)

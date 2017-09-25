#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, AvalancheSchedulerSvc, AlgResourcePool

# convenience machinery for assembling custom graphs of algorithm precedence rules (w/ CPUCrunchers as algorithms)
from GaudiHive import precedence

# metaconfig
evtslots = 1
evtMax = 1
algosInFlight = 4


InertMessageSvc(OutputLevel=INFO)

whiteboard = HiveWhiteBoard("EventDataSvc",
                            EventSlots=evtslots,
                            OutputLevel=INFO)

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=DEBUG)

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=algosInFlight,
                                  OutputLevel=DEBUG,
                                  #Optimizer = "DRE",
                                  PreemptiveIOBoundTasks=False,
                                  DumpIntraEventDynamics=False)

AlgResourcePool(OutputLevel=DEBUG)

#timeValue = precedence.UniformTimeValue(algoAvgTime=0.2)
timeValue = precedence.RealTimeValue(path="atlas/mcreco/averageTiming.mcreco.TriggerOff.json",
                                     defaultTime=0.0)
ifIObound = precedence.UniformBooleanValue(False)
#ifIObound = precedence.RndBiased10BooleanValue()

sequencer = precedence.CruncherSequence(timeValue, ifIObound, sleepFraction=0.0,
                                        cfgPath="atlas/mcreco/cf.mcreco.TriggerOff.graphml",
                                        dfgPath="atlas/mcreco/df.mcreco.TriggerOff.3rdEvent.graphml",
                                        topSequencer='AthSequencer/AthMasterSeq').get()

ApplicationMgr(EvtMax=evtMax,
               EvtSel='NONE',
               ExtSvc=[whiteboard],
               EventLoop=slimeventloopmgr,
               TopAlg=[sequencer],
               MessageSvcType="InertMessageSvc",
               OutputLevel=DEBUG)

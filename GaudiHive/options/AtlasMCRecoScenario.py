#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, ForwardSchedulerSvc, AlgResourcePool

# convenience machinery for assembling custom graphs of algorithm precedence rules (w/ CPUCrunchers as algorithms)
from GaudiHive import precedence

# metaconfig
evtslots = 1
evtMax = 3
algosInFlight = 8


InertMessageSvc(OutputLevel=INFO)

whiteboard   = HiveWhiteBoard("EventDataSvc",
                              EventSlots = evtslots,
                              OutputLevel = INFO)

slimeventloopmgr = HiveSlimEventLoopMgr(OutputLevel=DEBUG)

scheduler = ForwardSchedulerSvc(MaxEventsInFlight = evtslots,
                                MaxAlgosInFlight = algosInFlight,
                                ThreadPoolSize = algosInFlight,
                                OutputLevel = DEBUG,
                                useGraphFlowManagement = True,
                                DataFlowManagerNext = True,
                                #Optimizer = "DRE",
                                PreemptiveIOBoundTasks = False,
                                DumpIntraEventDynamics = False)

AlgResourcePool(OutputLevel = DEBUG)

#timeValue = precedence.UniformTimeValue(algoAvgTime=0.2)
timeValue = precedence.RealTimeValue(path = "atlas/mcreco/averageTiming.mcreco.TriggerOff.json",
                                     defaultTime = 0.0)
ifIObound = precedence.UniformBooleanValue(False)
#ifIObound = precedence.RndBiased10BooleanValue()

sequencer = precedence.CruncherSequence(timeValue, ifIObound, sleepFraction=0.0,
                                        cfgPath = "atlas/mcreco/cf.mcreco.TriggerOff.graphml",
                                        dfgPath = "atlas/mcreco/df.mcreco.TriggerOff.3rdEvent.graphml",
                                        topSequencer = 'AthSequencer/AthMasterSeq',
                                        showStat=True).get()

ApplicationMgr( EvtMax = evtMax,
                EvtSel = 'NONE',
                ExtSvc = [whiteboard],
                EventLoop = slimeventloopmgr,
                TopAlg = [sequencer],
                MessageSvcType = "InertMessageSvc",
                OutputLevel = DEBUG)

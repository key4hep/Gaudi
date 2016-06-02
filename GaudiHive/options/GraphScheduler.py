#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, ForwardSchedulerSvc, AlgResourcePool

from GaudiHive.precedence import precedence  # custom precedence graph, composed of CPUCrunchers

# metaconfig
evtslots = 1
evtMax = 1
cardinality = 1
algosInFlight = 4
algoAvgTime = 0.02


InertMessageSvc(OutputLevel=INFO)

whiteboard   = HiveWhiteBoard("EventDataSvc",
                              EventSlots = evtslots,
                              OutputLevel = INFO)

slimeventloopmgr = HiveSlimEventLoopMgr(OutputLevel=INFO)

scheduler = ForwardSchedulerSvc(MaxEventsInFlight = evtslots,
                                MaxAlgosInFlight = algosInFlight,
                                ThreadPoolSize = algosInFlight,
                                OutputLevel = DEBUG,
                                useGraphFlowManagement = True,
                                DataFlowManagerNext = True,
                                Optimizer = "DRE",
                                PreemptiveIOBoundTasks = False,
                                DumpIntraEventDynamics = False)

AlgResourcePool(OutputLevel = DEBUG)

timeValue = precedence.UniformTimeValue(algoAvgTime)
ifIObound = precedence.UniformBooleanValue(False)
#ifIObound = precedence.RndBiased10BooleanValue()

sequencer = precedence.CruncherSequence(timeValue, ifIObound, sleepFraction=0.0).get()

ApplicationMgr( EvtMax = evtMax,
                EvtSel = 'NONE',
                ExtSvc = [whiteboard],
                EventLoop = slimeventloopmgr,
                TopAlg = [sequencer],
                MessageSvcType = "InertMessageSvc",
                OutputLevel = INFO)

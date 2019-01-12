#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, AvalancheSchedulerSvc, IOBoundAlgSchedulerSvc

# convenience machinery for assembling custom graphs of algorithm precedence rules (w/ CPUCrunchers as algorithms)
from GaudiHive import precedence

# metaconfig
evtslots = 1
evtMax = 10
algosInFlight = 4


InertMessageSvc(OutputLevel=INFO)

whiteboard = HiveWhiteBoard("EventDataSvc",
                            EventSlots=evtslots,
                            OutputLevel=INFO)

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=INFO)

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=algosInFlight,
                                  OutputLevel=VERBOSE,
                                  PreemptiveIOBoundTasks=True,
                                  MaxIOBoundAlgosInFlight=50,
                                  DumpIntraEventDynamics=True)

IOBoundAlgSchedulerSvc(OutputLevel=INFO)

#timeValue = precedence.UniformTimeValue(avgRuntime=0.1)
timeValue = precedence.RealTimeValue(path="cms/reco/algs-time.json",
                                     defaultTime=0.0)
#ifIObound = precedence.UniformBooleanValue(False)
ifIObound = precedence.RndBiasedBooleanValue(
    pattern={True: 17, False: 152}, seed=1)

sequencer = precedence.CruncherSequence(timeValue, ifIObound, sleepFraction=0.9,
                                        cfgPath="cms/reco/cf.graphml",
                                        dfgPath="cms/reco/df.graphml",
                                        topSequencer='AthSequencer/AthRegSeq').get()

ApplicationMgr(EvtMax=evtMax,
               EvtSel='NONE',
               ExtSvc=[whiteboard],
               EventLoop=slimeventloopmgr,
               TopAlg=[sequencer],
               MessageSvcType="InertMessageSvc",
               OutputLevel=INFO)

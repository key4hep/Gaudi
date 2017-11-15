#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import (HiveWhiteBoard, HiveSlimEventLoopMgr,
                           AvalancheSchedulerSvc, PrecedenceSvc, TimelineSvc)

# convenience machinery for assembling custom graphs of algorithm
# precedence rules (w/ CPUCrunchers as algorithms)
from GaudiHive import precedence

# metaconfig
evtslots = 1
evtMax = 1
algosInFlight = 1
enableTimeline = True


InertMessageSvc(OutputLevel=INFO)

##########################################################################
# test the task precedence rules and trace dumping mechanisms ############
##########################################################################
PrecedenceSvc(DumpPrecedenceRules=True,
              DumpPrecedenceTrace=True,
              OutputLevel=DEBUG)
TimelineSvc(RecordTimeline=enableTimeline,
            OutputLevel=DEBUG)
##########################################################################

whiteboard = HiveWhiteBoard("EventDataSvc",
                            EventSlots=evtslots,
                            OutputLevel=INFO)

slimeventloopmgr = HiveSlimEventLoopMgr(SchedulerName="AvalancheSchedulerSvc",
                                        OutputLevel=DEBUG)

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=algosInFlight,
                                  OutputLevel=DEBUG)


timeValue = precedence.RealTimeValue(path="atlas/mcreco/averageTiming.mcreco.TriggerOff.json",
                                     defaultTime=0.0)
ifIObound = precedence.UniformBooleanValue(False)

sequencer = precedence.CruncherSequence(timeValue, ifIObound, sleepFraction=0.0,
                                        cfgPath="atlas/mcreco/cf.mcreco.TriggerOff.graphml",
                                        dfgPath="atlas/mcreco/df.mcreco.TriggerOff.3rdEvent.graphml",
                                        topSequencer='AthSequencer/AthMasterSeq',
                                        timeline=enableTimeline).get()

ApplicationMgr(EvtMax=evtMax,
               EvtSel='NONE',
               ExtSvc=[whiteboard],
               EventLoop=slimeventloopmgr,
               TopAlg=[sequencer],
               MessageSvcType="InertMessageSvc",
               OutputLevel=INFO)

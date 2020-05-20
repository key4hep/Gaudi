#!/usr/bin/env gaudirun.py
#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, AvalancheSchedulerSvc, CPUCrunchSvc

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
    PreemptiveBlockingTasks=True,
    MaxBlockingAlgosInFlight=50,
    DumpIntraEventDynamics=True)

CPUCrunchSvc(shortCalib=True)

#timeValue = precedence.UniformTimeValue(avgRuntime=0.1)
timeValue = precedence.RealTimeValue(
    path="atlas/mcreco/averageTiming.mcreco.TriggerOff.json", defaultTime=0.0)
#ifBlocking = precedence.UniformBooleanValue(False)
ifBlocking = precedence.RndBiasedBooleanValue(
    pattern={
        True: 17,
        False: 152
    }, seed=1)

sequencer = precedence.CruncherSequence(
    timeValue,
    ifBlocking,
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

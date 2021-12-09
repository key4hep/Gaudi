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

from Configurables import (
    AlgResourcePool,
    AvalancheSchedulerSvc,
    CPUCrunchSvc,
    HiveSlimEventLoopMgr,
    HiveWhiteBoard,
)
from Gaudi.Configuration import *

# convenience machinery for assembling custom graphs of algorithm precedence rules (w/ CPUCrunchers as algorithms)
from GaudiHive import precedence

# metaconfig
evtslots = 1
evtMax = 1
algosInFlight = 4
threads = 4

InertMessageSvc(OutputLevel=INFO)

whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots, OutputLevel=INFO)

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=DEBUG
)

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=threads, OutputLevel=DEBUG)

AlgResourcePool(OutputLevel=DEBUG)

CPUCrunchSvc(shortCalib=True)

# timeValue = precedence.UniformTimeValue(avgRuntime=0.2)
timeValue = precedence.RealTimeValue(
    path="atlas/mcreco/averageTiming.mcreco.TriggerOff.json", defaultTime=0.0
)
ifIObound = precedence.UniformBooleanValue(False)

sequencer = precedence.CruncherSequence(
    timeValue,
    ifIObound,
    sleepFraction=0.0,
    cfgPath="atlas/mcreco/cf.mcreco.TriggerOff.graphml",
    dfgPath="atlas/mcreco/df.mcreco.TriggerOff.3rdEvent.graphml",
    topSequencer="AthSequencer/AthMasterSeq",
    cardinality=algosInFlight,
).get()

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel="NONE",
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[sequencer],
    MessageSvcType="InertMessageSvc",
    OutputLevel=DEBUG,
)

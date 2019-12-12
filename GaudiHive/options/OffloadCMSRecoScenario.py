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
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, AvalancheSchedulerSvc, IOBoundAlgSchedulerSvc

# convenience machinery for assembling custom graphs of algorithm precedence rules (w/ CPUCrunchers as algorithms)
from GaudiHive import precedence

# metaconfig
evtslots = 1
evtMax = 5
algosInFlight = 4

whiteboard = HiveWhiteBoard(
    "EventDataSvc", EventSlots=evtslots, OutputLevel=INFO)

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=INFO)

AvalancheSchedulerSvc(
    ThreadPoolSize=algosInFlight,
    OutputLevel=DEBUG,
    PreemptiveIOBoundTasks=True,
    MaxIOBoundAlgosInFlight=50)

IOBoundAlgSchedulerSvc(OutputLevel=INFO)

#timeValue = precedence.UniformTimeValue(avgRuntime=0.1)
timeValue = precedence.RealTimeValue(
    path="cms/reco/algs-time.json", defaultTime=0.0)

#ifIObound = precedence.UniformBooleanValue(False)
# the CMS reco scenario has 707 algorithms in total
ifIObound = precedence.RndBiasedBooleanValue(
    pattern={
        True: 70,
        False: 637
    }, seed=1)

sequencer = precedence.CruncherSequence(
    timeValue,
    ifIObound,
    sleepFraction=0.9,
    cfgPath="cms/reco/cf.graphml",
    dfgPath="cms/reco/df.graphml",
    topSequencer='TopSequencer').get()

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel='NONE',
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[sequencer],
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO)

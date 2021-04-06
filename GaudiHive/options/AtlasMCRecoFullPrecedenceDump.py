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
from Configurables import (HiveWhiteBoard, HiveSlimEventLoopMgr,
                           AvalancheSchedulerSvc, PrecedenceSvc, TimelineSvc)

# convenience machinery for assembling custom graphs of algorithm
# precedence rules (w/ CPUCrunchers as algorithms)
from GaudiHive import precedence

# metaconfig
evtslots = 2
evtMax = 4
algosInFlight = 2
enableTimeline = True

InertMessageSvc(OutputLevel=INFO)

##########################################################################
# test the task precedence rules and trace dumping mechanisms ############
##########################################################################
PrecedenceSvc(
    DumpPrecedenceRules=True, DumpPrecedenceTrace=True, OutputLevel=DEBUG)
TimelineSvc(RecordTimeline=enableTimeline, OutputLevel=DEBUG)
##########################################################################

whiteboard = HiveWhiteBoard(
    "EventDataSvc", EventSlots=evtslots, OutputLevel=INFO)

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=DEBUG)

scheduler = AvalancheSchedulerSvc(
    ThreadPoolSize=algosInFlight, OutputLevel=DEBUG)

timeValue = precedence.RealTimeValue(
    path="atlas/q431/time.r2a.json", defaultTime=0.1)
ifIObound = precedence.UniformBooleanValue(False)

sequencer = precedence.CruncherSequence(
    timeValue,
    ifIObound,
    sleepFraction=0.0,
    cfgPath="atlas/q431/cf.r2a.graphml",
    dfgPath="atlas/q431/df.r2a.graphml",
    topSequencer='AthSequencer/AthMasterSeq',
    timeline=enableTimeline).get()

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel='NONE',
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[sequencer],
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO)

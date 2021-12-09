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
"""
The simplest possible configuration for preemptive scheduling of single blocking algorithm.
"""

from Configurables import (
    AvalancheSchedulerSvc,
    CPUCruncher,
    CPUCrunchSvc,
    HiveSlimEventLoopMgr,
    HiveWhiteBoard,
)
from Gaudi.Configuration import *

# metaconfig
evtMax = 7
evtslots = 3
algosInFlight = 0
blockingAlgosInFlight = 3

CPUCrunchSvc(shortCalib=True)

whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots, OutputLevel=INFO)

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=INFO
)

AvalancheSchedulerSvc(
    ThreadPoolSize=algosInFlight,
    PreemptiveBlockingTasks=True,
    MaxBlockingAlgosInFlight=blockingAlgosInFlight,
    OutputLevel=VERBOSE,
)

blockingAlg = CPUCruncher(
    name="BlockingAlg",
    avgRuntime=2.0,
    Cardinality=3,
    Blocking=True,  # tag algorithm as blocking
    SleepFraction=0.7,  # simulate blocking nature
    OutputLevel=DEBUG,
)

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel="NONE",
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[blockingAlg],
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO,
)

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
A test for a single algorithm with internal multithreading
"""

from Configurables import (
    AlgResourcePool,
    AvalancheSchedulerSvc,
    CPUCruncher,
    CPUCrunchSvc,
    HiveSlimEventLoopMgr,
    HiveWhiteBoard,
)
from Gaudi.Configuration import *

evtslots = 1
evtMax = 1
cardinality = 1
threads = 5  # Threading within alg is still limited by total thread pool

whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots)

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=DEBUG
)

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=threads, OutputLevel=VERBOSE)

AlgResourcePool(OutputLevel=DEBUG)

CPUCrunchSvc(shortCalib=True)

# Set up of CPU crunchers -------------------------------------------------------

a1 = CPUCruncher("A1")
a1.Cardinality = cardinality
a1.avgRuntime = 5.0
a1.NParallel = 5
a1.OutputLevel = DEBUG

# Application Manager ----------------------------------------------------------

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel="NONE",
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[a1],
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO,
)

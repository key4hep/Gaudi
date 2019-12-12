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
Find and attribute unmet data inputs as outputs to a Data Loader algorithm.
"""

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, AvalancheSchedulerSvc, CPUCruncher, CPUCrunchSvc

# metaconfig
evtslots = 1
evtMax = 3
algosInFlight = 1

whiteboard = HiveWhiteBoard(
    "EventDataSvc", EventSlots=evtslots, OutputLevel=INFO)

slimeventloopmgr = HiveSlimEventLoopMgr(SchedulerName="AvalancheSchedulerSvc")

AvalancheSchedulerSvc(
    ThreadPoolSize=algosInFlight, CheckDependencies=True, DataLoaderAlg="AlgA")

CPUCrunchSvc(shortCalib=True)

# Assemble the data flow graph
a1 = CPUCruncher("AlgA", Loader=True, OutputLevel=VERBOSE)

a2 = CPUCruncher("AlgB", OutputLevel=VERBOSE)
a2.inpKeys = ['/Event/A1']

a3 = CPUCruncher("AlgC", OutputLevel=VERBOSE)
a3.inpKeys = ['/Event/A2']

for a in [a1, a2, a3]:
    a.avgRuntime = .01

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel='NONE',
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[a1, a2, a3],
    MessageSvcType="InertMessageSvc",
    OutputLevel=DEBUG)

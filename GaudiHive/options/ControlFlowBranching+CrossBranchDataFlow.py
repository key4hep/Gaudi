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
Typically, an EVTREJECTED algorithm is used to "short-circuit" the execution flow within
the group of algorithms it belongs to. Within such early-terminated group, a false-negative
data dependency resolution can never occur since all downstream algorithms are skipped.
However, there might be other algorithms - outside of the early-terminated group - that depend
on data outputs of the EVTREJECTED algorithm. The test emulates this scenario.
"""

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, AvalancheSchedulerSvc, AlgResourcePool
from Configurables import GaudiSequencer, CPUCruncher, CPUCrunchSvc

# metaconfig
evtslots = 1
evtMax = 1
algosInFlight = 1

InertMessageSvc(OutputLevel=INFO)

whiteboard = HiveWhiteBoard(
    "EventDataSvc", EventSlots=evtslots, OutputLevel=INFO)

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=DEBUG)

AvalancheSchedulerSvc(ThreadPoolSize=algosInFlight, OutputLevel=DEBUG)

AlgResourcePool(OutputLevel=DEBUG)

CPUCrunchSvc(shortCalib=True)

# Assemble data flow graph
# algorithm that triggers an early exit from "Branch2"
a1 = CPUCruncher("AlgA", InvertDecision=True)
a1.outKeys = ['/Event/A']

a2 = CPUCruncher("AlgB")
a2.inpKeys = ['/Event/A']

a3 = CPUCruncher("AlgC")
a3.inpKeys = ['/Event/A']

for a in [a1, a2, a3]:
    a.avgRuntime = .01

# Assemble control flow graph
branch1 = GaudiSequencer("Branch1", ModeOR=False, ShortCircuit=False)
branch2 = GaudiSequencer(
    "Branch2", ModeOR=False, ShortCircuit=True, Sequential=True)

branch2.Members = [a1, a2]
branch1.Members = [branch2, a3]

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel='NONE',
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[branch1],
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO)

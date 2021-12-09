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
The options file models an intra-event stall in algorithm scheduling.
An early exit from a group of algorithms called 'EarlyExitBranch' is performed due to
an inverted CF decision sot that A2 is not run. This results in an unmet DF dependency
for a downstream algorithm A3, leading to the stall.
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
threads = 1

whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots)

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=DEBUG
)

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=threads, OutputLevel=VERBOSE)

AlgResourcePool(OutputLevel=DEBUG)

CPUCrunchSvc(shortCalib=True)

# Set up of CPU crunchers -------------------------------------------------------

a1 = CPUCruncher("A1")
# to skip algorithm 'A2', force early exit from the branch
# This leads to data flow stall
a1.InvertDecision = True

# this algorithm is not run due to early exit from the group it belongs to
a2 = CPUCruncher("A2")
a2.outKeys = ["/Event/a2"]

a3 = CPUCruncher("A3")
a3.inpKeys = ["/Event/a2"]

# Assemble control flow graph
branch = GaudiSequencer(
    "EarlyExitBranch", ModeOR=False, ShortCircuit=True, Sequential=True
)
branch.Members = [a1, a2]

for algo in [a1, a2, a3]:
    algo.Cardinality = cardinality
    algo.avgRuntime = 0.1

# Application Manager ----------------------------------------------------------

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel="NONE",
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[branch, a3],
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO,
)

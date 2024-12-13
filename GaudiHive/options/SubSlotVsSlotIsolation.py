#!/usr/bin/env gaudirun.py
#####################################################################################
# (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
"""
A test for isolating sub-slot data from the parent slot.
The main sequence has two steps of creating and running sub-events.

The ViewTester is an algorithm specifically designed to create sub-event
contexts, pass them to the scheduler, and report on the current context.

Seven instances of ViewTester are used as follows:
 - Algorithm A1 creates a sub-event context
   - Algorithms A2 and A3 run within the sub-event, sharing data
 - Algorithm A4 creates another sub-event contexts, and a data output
   - Algorithms A5 and A6 run within the new sub-event, but cannot access data from A4
 - Algorithm A7 runs in the whole event context, after the sub-events

The test should stall if isolation is working
"""

from Configurables import (
    AlgResourcePool,
    AvalancheSchedulerSvc,
    Gaudi__Sequencer,
    HiveSlimEventLoopMgr,
    HiveWhiteBoard,
    Test__ViewTester,
)
from Gaudi.Configuration import *

# metaconfig -------------------------------------------------------------------
# It's confortable to collect the relevant parameters at the top of the optionfile
evtslots = 1
evtMax = 1
cardinality = 1
threads = 1
viewsPerEvt = 1
# -------------------------------------------------------------------------------

# The configuration of the whiteboard ------------------------------------------
# It is useful to call it EventDataSvc to replace the usual data service with
# the whiteboard transparently.

whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots)

# -------------------------------------------------------------------------------

# Event Loop Manager -----------------------------------------------------------
# It's called slim since it has less functionalities overall than the good-old
# event loop manager. Here we just set its outputlevel to DEBUG.

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=DEBUG
)

# -------------------------------------------------------------------------------

# ForwardScheduler -------------------------------------------------------------
# We just decide how many algorithms in flight we want to have and how many
# threads in the pool. The default value is -1, which is for TBB equivalent
# to take over the whole machine.

scheduler = AvalancheSchedulerSvc(
    ThreadPoolSize=threads, OutputLevel=INFO, VerboseSubSlots=True
)

# -------------------------------------------------------------------------------

# Algo Resource Pool -----------------------------------------------------------
# Nothing special here, we just set the debug level.
AlgResourcePool(OutputLevel=DEBUG)

# -------------------------------------------------------------------------------

# Set up of the crunchers, daily business --------------------------------------

a1 = Test__ViewTester("A1")
a1.baseViewName = "viewOne"
a1.viewNumber = viewsPerEvt
a1.viewNodeName = "viewNodeOne"

a2 = Test__ViewTester("A2")
a2.viewNodeName = ""
a2.outKeys = ["/Event/a2"]

a3 = Test__ViewTester("A3")
a3.viewNodeName = ""
a3.inpKeys = ["/Event/a2"]  # Should be able to load a2 from the shared sub-slot

a4 = Test__ViewTester("A4")
a4.baseViewName = "viewTwo"
a4.viewNumber = viewsPerEvt
a4.viewNodeName = "viewNodeTwo"
a4.outKeys = ["/Event/a4"]

a5 = Test__ViewTester("A5")
a5.viewNodeName = ""
a5.inpKeys = ["/Event/a4"]  # Should not be able to load a4 from the "whole event" slot

a6 = Test__ViewTester("A6")
a6.viewNodeName = ""

a7 = Test__ViewTester("A7")
a7.viewNodeName = ""

for algo in [a1, a2, a3, a4, a5, a6, a7]:
    algo.Cardinality = cardinality
    algo.OutputLevel = DEBUG

viewNodeOne = Gaudi__Sequencer(
    "viewNodeOne", Members=[a2, a3], Sequential=False, ShortCircuit=False
)

viewNodeTwo = Gaudi__Sequencer(
    "viewNodeTwo", Members=[a5, a6], Sequential=False, ShortCircuit=False
)

createViewSeq = Gaudi__Sequencer(
    "createViewSeq",
    Members=[a1, viewNodeOne, a4, viewNodeTwo, a7],
    Sequential=True,
    OutputLevel=VERBOSE,
)

# Application Manager ----------------------------------------------------------
# We put everything together and change the type of message service

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel="NONE",
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[createViewSeq],
    MessageSvcType="InertMessageSvc",
)

# -------------------------------------------------------------------------------

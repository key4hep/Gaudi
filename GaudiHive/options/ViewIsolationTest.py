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
A test for issue 13 in gitlab.
Two different sets of sub-event contexts are created, and the
same algorithms (attached to a common control flow node) run in each

The ViewTester is an algorithm specifically designed to create sub-event
contexts, pass them to the scheduler, and report on the current context.

Five instances of ViewTester are used as follows:
 - Algorithm A1 creates two sub-event contexts
   - Algorithms A2 and A3 run within the sub-event contexts
 - Algorithm A4 creates two more sub-event contexts
   - Algorithms A2 and A3 run again, in the new contexts
 - Algorithm A5 runs in the whole event context, after the sub-events

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
evtMax = 10
cardinality = 1
threads = 1
viewsPerEvt = 2
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
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=INFO
)

# -------------------------------------------------------------------------------

# ForwardScheduler -------------------------------------------------------------
# We just decide how many algorithms in flight we want to have and how many
# threads in the pool. The default value is -1, which is for TBB equivalent
# to take over the whole machine.

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=threads, OutputLevel=INFO)

# -------------------------------------------------------------------------------

# Algo Resource Pool -----------------------------------------------------------
# Nothing special here, we just set the debug level.
AlgResourcePool(OutputLevel=INFO)

# -------------------------------------------------------------------------------

# Set up of the crunchers, daily business --------------------------------------

a1 = Test__ViewTester("A1")
a1.baseViewName = "viewOne"
a1.viewNumber = viewsPerEvt
a1.viewNodeName = "viewNodeOne"

a2 = Test__ViewTester("A2")
a2.viewNodeName = ""

a3 = Test__ViewTester("A3")
a3.viewNodeName = ""

a4 = Test__ViewTester("A4")
a4.baseViewName = "viewTwo"
a4.viewNumber = viewsPerEvt
a4.viewNodeName = "viewNodeTwo"

a5 = Test__ViewTester("A5")
a5.viewNodeName = ""

for algo in [a1, a2, a3, a4, a5]:
    algo.Cardinality = cardinality
    algo.OutputLevel = INFO

extraNode = Gaudi__Sequencer(
    "extraNode", Members=[a2, a3], Sequential=True, OutputLevel=INFO
)

viewNodeOne = Gaudi__Sequencer(
    "viewNodeOne",
    Members=[extraNode],
    Sequential=False,
    ShortCircuit=False,
    OutputLevel=INFO,
)

viewNodeTwo = Gaudi__Sequencer(
    "viewNodeTwo",
    Members=[extraNode],
    Sequential=False,
    ShortCircuit=False,
    OutputLevel=INFO,
)

createViewSeq = Gaudi__Sequencer(
    "createViewSeq",
    Members=[a1, viewNodeOne, a4, viewNodeTwo, a5],
    Sequential=True,
    OutputLevel=INFO,
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

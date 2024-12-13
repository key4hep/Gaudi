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
A test for control flow scheduling within sub-event contexts.
The sub-event control flow node has a child node attached.

The ViewTester is an algorithm specifically designed to create sub-event
contexts, pass them to the scheduler, and report on the current context.

Six instances of ViewTester are used as follows:
 - Algorithm A1 creates two sub-event contexts
   - Algorithms A2 and A3 run within the sub-event contexts
     - Algorithms B1 and B2 run within the sub-event contexts,
       on a child control flow node
 - Algorithm A4 runs in the whole event context, after the sub-events

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
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=DEBUG
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
AlgResourcePool(OutputLevel=DEBUG)

# -------------------------------------------------------------------------------

# Set up of the crunchers, daily business --------------------------------------

a1 = Test__ViewTester("A1")
a1.baseViewName = "view"
a1.viewNumber = viewsPerEvt
a1.viewNodeName = "viewNode"

a2 = Test__ViewTester("A2")
a2.viewNodeName = ""

a3 = Test__ViewTester("A3")
a3.viewNodeName = ""

a4 = Test__ViewTester("A4")
a4.viewNodeName = ""

b1 = Test__ViewTester("B1")
b1.viewNodeName = ""

b2 = Test__ViewTester("B2")
b2.viewNodeName = ""

for algo in [a1, a2, a3, a4, b1, b2]:
    algo.Cardinality = cardinality
    algo.OutputLevel = DEBUG

nodeInView = Gaudi__Sequencer(
    "nodeInView", Members=[b1, b2], Sequential=False, OutputLevel=VERBOSE
)

viewNode = Gaudi__Sequencer(
    "viewNode", Members=[a2, nodeInView, a3], Sequential=False, OutputLevel=VERBOSE
)

createViewSeq = Gaudi__Sequencer(
    "createViewSeq", Members=[a1, viewNode, a4], Sequential=True, OutputLevel=VERBOSE
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

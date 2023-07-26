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
A test for a control flow bug arising from empty graph nodes

An empty node with ModeOR=True will return a default decision of False, without any algs needing evaluation
This can have the effect of short-circuiting/early return from its parent node, while also continuing 
evaluation within that parent - see https://gitlab.cern.ch/gaudi/Gaudi/-/issues/135

In this test, A2 will run unless the bug has been fixed
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

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=threads, OutputLevel=VERBOSE)

# -------------------------------------------------------------------------------

# Algo Resource Pool -----------------------------------------------------------
# Nothing special here, we just set the debug level.
AlgResourcePool(OutputLevel=INFO)

# -------------------------------------------------------------------------------

# Set up of the crunchers, daily business --------------------------------------

a1 = Test__ViewTester("A1")
a1.Cardinality = cardinality
a1.OutputLevel = INFO
a1.viewNodeName = ""
a2 = Test__ViewTester("A2")
a2.Cardinality = cardinality
a2.OutputLevel = INFO
a2.viewNodeName = ""

emptySeq = Gaudi__Sequencer(
    "emptySeq",
    Members=[],
    Sequential=False,
    ModeOR=True,
    ShortCircuit=False,
    OutputLevel=INFO,
)

topSeq = Gaudi__Sequencer(
    "topSeq",
    # Members=[emptySeq, a1], #This finds a different bug/quirk
    Members=[a1, emptySeq, a2],
    Sequential=True,
    ModeOR=False,
    ShortCircuit=True,
    OutputLevel=INFO,
)

# Application Manager ----------------------------------------------------------
# We put everything together and change the type of message service

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel="NONE",
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[topSeq],
    MessageSvcType="InertMessageSvc",
)

# -------------------------------------------------------------------------------

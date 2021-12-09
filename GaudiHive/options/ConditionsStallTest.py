#!/usr/bin/env gaudirun.py
#####################################################################################
# (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
"""
A test to demonstrate stalling for a conditions algorithm

 - Control flow requires that Alg C run after Alg B
 - Alg B requires conditions data produced by Alg A
 - However, Alg A requires the output from Alg C, and thus the job will stall

"""
from Configurables import (
    AlgResourcePool,
    AvalancheSchedulerSvc,
    HiveSlimEventLoopMgr,
    HiveWhiteBoard,
    Test__ViewTester,
)
from Gaudi.Configuration import *

# metaconfig -------------------------------------------------------------------
# It's confortable to collect the relevant parameters at the top of the optionfile
evtslots = 1
evtMax = 10
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

scheduler = AvalancheSchedulerSvc(
    ThreadPoolSize=threads, OutputLevel=INFO, CheckDependencies=True, DataLoaderAlg=""
)

# -------------------------------------------------------------------------------

# Algo Resource Pool -----------------------------------------------------------
# Nothing special here, we just set the debug level.
AlgResourcePool(OutputLevel=INFO)

# -------------------------------------------------------------------------------

# Conditions service -----------------------------------------------------------
# This declares algorithms or data to be part of the "conditions realm"
# They are detached from the regular CF graph
from Configurables import Gaudi__Examples__Conditions__CondSvc as CS

condSvc = CS(name="CondSvc", Algs=["AlgA"], Data=["/Event/A1"])

# -------------------------------------------------------------------------------

# Set up of the crunchers, daily business --------------------------------------

a1 = Test__ViewTester("AlgA", OutputLevel=INFO)
a1.outKeys = ["/Event/A1"]
a1.inpKeys = ["/Event/A2"]

a2 = Test__ViewTester("AlgB", OutputLevel=INFO)
a2.inpKeys = ["/Event/A1"]

a3 = Test__ViewTester("AlgC", OutputLevel=INFO)
a3.outKeys = ["/Event/A2"]

algSeq = GaudiSequencer(
    "algSeq", Members=[a1, a2, a3], Sequential=True, OutputLevel=INFO
)

# Application Manager ----------------------------------------------------------
# We put everything together and change the type of message service

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel="NONE",
    ExtSvc=[whiteboard, condSvc],
    EventLoop=slimeventloopmgr,
    TopAlg=[algSeq],
    MessageSvcType="InertMessageSvc",
)

# -------------------------------------------------------------------------------

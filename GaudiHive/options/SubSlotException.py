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
'''
A test for an incorrect handling of exceptions from algorithms running in sub-slots

Throwing an exception causes the event to be marked as failed.
It also means that the part of the code that updates the algorithm state is bypassed.
Since the AlgExecStateSvc does not (currently) understand sub-slots,
if the exception is thrown by an alg in sub-slot 2, the state for that same alg
in sub-slot 1 is retrieved.

So, it is possible to have a failed event, without any algorihms in ERROR state.
The scheduler does not have handling for this, and hangs.

'''
from Gaudi.Configuration import *
from Configurables import (HiveWhiteBoard, HiveSlimEventLoopMgr,
                           AvalancheSchedulerSvc, AlgResourcePool, CPUCruncher,
                           GaudiSequencer, Test__ViewTester,
                           GaudiTesting__StopLoopAlg)

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
# event loop manager. Here we just set its outputlevel to INFO.

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=INFO)

# -------------------------------------------------------------------------------

# ForwardScheduler -------------------------------------------------------------
# We just decide how many algorithms in flight we want to have and how many
# threads in the pool. The default value is -1, which is for TBB equivalent
# to take over the whole machine.

scheduler = AvalancheSchedulerSvc(
    ThreadPoolSize=threads, OutputLevel=INFO, VerboseSubSlots=True)

# -------------------------------------------------------------------------------

# Algo Resource Pool -----------------------------------------------------------
# Nothing special here, we just set the debug level.
AlgResourcePool(OutputLevel=INFO)

# -------------------------------------------------------------------------------

# Set up of the crunchers, daily business --------------------------------------

a1 = Test__ViewTester("A1")
a1.baseViewName = 'view'
a1.viewNumber = viewsPerEvt
a1.viewNodeName = 'viewNode'

a2 = Test__ViewTester("A2")
a2.viewNodeName = ''

# EventCount is tracked by a private member of the algorithm, so increments whenever it is run
# EventCount = 2 corresponds to the 1st view of the 2nd event, giving a correctly-handled exception
# EventCount = 3 corresponds to the 2nd view of the 2nd event, causing the hang
a3 = GaudiTesting__StopLoopAlg("A3", EventCount=3, Mode="exception")

a4 = Test__ViewTester("A4")
a4.viewNodeName = ''

for algo in [a1, a2, a3, a4]:
    algo.Cardinality = cardinality
    algo.OutputLevel = INFO

viewNode = GaudiSequencer(
    "viewNode",
    Members=[a2, a3],
    Sequential=False,
    ShortCircuit=False,
    OutputLevel=INFO)

createViewSeq = GaudiSequencer(
    "createViewSeq",
    Members=[a1, viewNode, a4],
    Sequential=True,
    OutputLevel=INFO)

# Application Manager ----------------------------------------------------------
# We put everything together and change the type of message service

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel='NONE',
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[createViewSeq],
    MessageSvcType="InertMessageSvc")

# -------------------------------------------------------------------------------

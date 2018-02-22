#!/usr/bin/env gaudirun.py

'''
A test for basic functionality of sub-event scheduling.
Algorithms A2 and A3 should run twice per event, in sub-event contexts.

The ViewTester is an algorithm specifically designed to create sub-event
contexts, pass them to the scheduler, and report on the current context.

Four instances of ViewTester are used as follows:
 - Algorithm A1 creates two sub-event contexts
   - Algorithms A2 and A3 run within the sub-event contexts
 - Algorithm A4 runs in the whole event context, after the sub-events

'''
from Gaudi.Configuration import *
from Configurables import (HiveWhiteBoard, HiveSlimEventLoopMgr,
                           AvalancheSchedulerSvc, AlgResourcePool,
                           CPUCruncher, GaudiSequencer, Test__ViewTester)

# metaconfig -------------------------------------------------------------------
# It's confortable to collect the relevant parameters at the top of the optionfile
evtslots = 1
evtMax = 10
cardinality = 1
threads = 1
viewsPerEvt = 2
#-------------------------------------------------------------------------------

# The configuration of the whiteboard ------------------------------------------
# It is useful to call it EventDataSvc to replace the usual data service with
# the whiteboard transparently.

whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots)

#-------------------------------------------------------------------------------

# Event Loop Manager -----------------------------------------------------------
# It's called slim since it has less functionalities overall than the good-old
# event loop manager. Here we just set its outputlevel to DEBUG.

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=INFO)

#-------------------------------------------------------------------------------

# ForwardScheduler -------------------------------------------------------------
# We just decide how many algorithms in flight we want to have and how many
# threads in the pool. The default value is -1, which is for TBB equivalent
# to take over the whole machine.

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=threads, OutputLevel=INFO)

#-------------------------------------------------------------------------------

# Algo Resource Pool -----------------------------------------------------------
# Nothing special here, we just set the debug level.
AlgResourcePool(OutputLevel=INFO)

#-------------------------------------------------------------------------------

# Set up of the crunchers, daily business --------------------------------------

a1 = Test__ViewTester("A1")
a1.baseViewName = 'view'
a1.viewNumber = viewsPerEvt
a1.viewNodeName = 'viewNode'

a2 = Test__ViewTester("A2")
a2.viewNodeName = ''

a3 = Test__ViewTester("A3")
a3.viewNodeName = ''

a4 = Test__ViewTester("A4")
a4.viewNodeName = ''

for algo in [a1, a2, a3, a4]:
    algo.Cardinality = cardinality
    algo.OutputLevel = INFO

viewNode = GaudiSequencer("viewNode",
                          Members=[a2, a3],
                          Sequential=False,
                          OutputLevel=INFO)

createViewSeq = GaudiSequencer("createViewSeq",
                               Members=[a1, viewNode, a4],
                               Sequential=True,
                               OutputLevel=INFO)

# Application Manager ----------------------------------------------------------
# We put everything together and change the type of message service

ApplicationMgr(EvtMax=evtMax,
               EvtSel='NONE',
               ExtSvc=[whiteboard],
               EventLoop=slimeventloopmgr,
               TopAlg=[createViewSeq],
               MessageSvcType="InertMessageSvc")

#-------------------------------------------------------------------------------

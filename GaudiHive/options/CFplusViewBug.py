#!/usr/bin/env gaudirun.py
'''
A deliberately badly-configured job to demonstrate a crash in
sub-event scheduling.

The ViewTester is an algorithm specifically designed to create sub-event
contexts, pass them to the scheduler, and report on the current context.

The re-use of filter_alg causes the CF to "skip over" filter_alg2
This would then cause sub-events to be scheduled, but with the
"ShortCircuit" property of the hltStep node set True, the event can
be deleted before the sub-events have completed, causing the crash.

There is now protection in AvalancheScheduler against this crash,
but note that re-using algs in this way is still expected to cause
surprising CF behaviour.
'''
from Gaudi.Configuration import *
from Configurables import (HiveWhiteBoard, HiveSlimEventLoopMgr,
                           AvalancheSchedulerSvc, AlgResourcePool, CPUCruncher,
                           GaudiSequencer, Test__ViewTester)

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
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=INFO)

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

filter_alg = CPUCruncher("filter_alg")
filter_alg.InvertDecision = False
filter_alg.avgRuntime = 0.0

filter_alg2 = CPUCruncher("filter_alg2")
filter_alg2.InvertDecision = True
filter_alg2.avgRuntime = 0.0

view_make_alg = Test__ViewTester("view_make_alg")
view_make_alg.baseViewName = 'view'
view_make_alg.viewNumber = viewsPerEvt
view_make_alg.viewNodeName = 'view_test_node'

view_test_alg = CPUCruncher("view_test_alg")
view_test_alg.InvertDecision = False
view_test_alg.avgRuntime = 1.0

for algo in [filter_alg, filter_alg2, view_make_alg, view_test_alg]:
    algo.Cardinality = cardinality
    algo.OutputLevel = INFO

view_test_node = GaudiSequencer(
    "view_test_node",
    Members=[view_test_alg],
    Sequential=False,
    ShortCircuit=False,
    OutputLevel=INFO)

view_make_node = GaudiSequencer(
    "view_make_node",
    Members=[filter_alg, view_make_alg, view_test_node],
    Sequential=True,
    OutputLevel=INFO)

hltStep = GaudiSequencer(
    "hltStep",
    Members=[filter_alg, filter_alg2, view_make_node],
    Sequential=True,
    ShortCircuit=True,
    ModeOR=False,
    OutputLevel=INFO)

# Application Manager ----------------------------------------------------------
# We put everything together and change the type of message service

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel='NONE',
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[hltStep],
    MessageSvcType="InertMessageSvc")

# -------------------------------------------------------------------------------

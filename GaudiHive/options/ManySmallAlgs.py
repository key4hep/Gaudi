#!/usr/bin/env gaudirun.py
'''
A totally synthetic test of scheduler performance, with a large number of CF nodes and algorithms, but each algorithm does no work.

In the flat mode (nested=False) 1000 algorithms are added directly to the base CF node as a baseline measurement.

In nested mode the algorithms are grouped into sets of under a parent CF node.
Those nodes are then given 2 different CF node parents (20 in total) which include a prescale algorithm.

In sequential mode (sequence=True) the prescale algorithms will randomly deactivate CF nodes at a frequency given by filterPass
'''
from Gaudi.Configuration import *
from Configurables import (HiveWhiteBoard, HiveSlimEventLoopMgr,
                           AvalancheSchedulerSvc, AlgResourcePool, Prescaler,
                           GaudiSequencer, Test__ViewTester)

# metaconfig -------------------------------------------------------------------
# It's confortable to collect the relevant parameters at the top of the optionfile
evtslots = 5
evtMax = 1000
cardinality = 10
threads = 5
nested = True
sequence = True
filterPass = 50.0

# -------------------------------------------------------------------------------

# The configuration of the whiteboard ------------------------------------------
# It is useful to call it EventDataSvc to replace the usual data service with
# the whiteboard transparently.

whiteboard = HiveWhiteBoard(
    "EventDataSvc", EventSlots=evtslots, OutputLevel=FATAL)

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

scheduler = AvalancheSchedulerSvc(
    ThreadPoolSize=threads,
    ShowControlFlow=False,
    ShowDataDependencies=False,
    OutputLevel=INFO)

# -------------------------------------------------------------------------------

# Algo Resource Pool -----------------------------------------------------------
# Nothing special here, we just set the debug level.
AlgResourcePool(OutputLevel=FATAL)

# -------------------------------------------------------------------------------

# Flat structure
allAlgs = []
for i in range(1000):
    alg = Prescaler("alg" + str(i))
    alg.OutputLevel = FATAL
    alg.Cardinality = cardinality
    alg.PercentPass = 100.0
    allAlgs.append(alg)

baseSeq = GaudiSequencer(
    "baseSeq",
    Members=allAlgs,
    Sequential=sequence,
    ModeOR=True,
    ShortCircuit=False,
    OutputLevel=FATAL)

# Nested structure
allSeqs = []
allSeqSeqs = []
if nested:
    for i in range(100):
        seqAlgs = allAlgs[10 * i:10 * (i + 1)]
        seq = GaudiSequencer(
            "seq" + str(i),
            Members=seqAlgs,
            Sequential=sequence,
            ModeOR=False,
            ShortCircuit=True,
            OutputLevel=FATAL)
        allSeqs.append(seq)

    for i in range(10):
        seqSeqs = allSeqs[10 * i:10 * (i + 1)]

        filterAlg = Prescaler("filterAlg" + str(i))
        filterAlg.OutputLevel = FATAL
        filterAlg.Cardinality = cardinality
        filterAlg.PercentPass = filterPass

        seq = GaudiSequencer(
            "seqSeq" + str(i),
            Members=[filterAlg] + seqSeqs,
            Sequential=sequence,
            ModeOR=False,
            ShortCircuit=True,
            OutputLevel=FATAL)
        allSeqSeqs.append(seq)

    for i in range(9):
        seqSeqs = allSeqs[5 + (10 * i):15 + (10 * i)]

        filterAlg = Prescaler("filterAlg" + str(i + 10))
        filterAlg.OutputLevel = FATAL
        filterAlg.Cardinality = cardinality
        filterAlg.PercentPass = filterPass

        seq = GaudiSequencer(
            "seqSeq" + str(i + 10),
            Members=[filterAlg] + seqSeqs,
            Sequential=sequence,
            ModeOR=False,
            ShortCircuit=True,
            OutputLevel=FATAL)
        allSeqSeqs.append(seq)

    baseSeq.Members = allSeqSeqs

# Application Manager ----------------------------------------------------------
# We put everything together and change the type of message service

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel='NONE',
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[baseSeq],
    MessageSvcType="InertMessageSvc")

# -------------------------------------------------------------------------------

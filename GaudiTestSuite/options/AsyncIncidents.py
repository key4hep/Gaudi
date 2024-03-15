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
#
# Copied from ForwardSchedulerControlFlowTest.py to setup asynchronous
# incident service test and example
#
#

from Configurables import (
    AlgResourcePool,
    AvalancheSchedulerSvc,
    CPUCruncher,
    CPUCrunchSvc,
    Gaudi__Sequencer,
    HiveSlimEventLoopMgr,
    HiveWhiteBoard,
    IncidentAsyncTestAlg,
    IncidentAsyncTestSvc,
    IncidentProcAlg,
    IncidentSvc,
)
from Gaudi.Configuration import *

msgFmt = "% F%40W%S%4W%s%e%7W%R%T %0W%M"
msgSvc = InertMessageSvc("MessageSvc", OutputLevel=INFO)
msgSvc.Format = msgFmt
ApplicationMgr().SvcMapping.append(msgSvc)

IncidentSvc(OutputLevel=DEBUG)
CPUCrunchSvc(shortCalib=True)

# metaconfig
evtslots = 5
evtMax = 20
cardinality = 10
algosInFlight = 10

whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots)

slimeventloopmgr = HiveSlimEventLoopMgr(OutputLevel=INFO)

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=algosInFlight, OutputLevel=DEBUG)

AlgResourcePool(OutputLevel=INFO)
# Async Incident svc processing algs to be added at the beginning and
# at the end of event graph to process incidents fired in the context
# of the given event

EventLoopInitProcAlg = IncidentProcAlg("EventLoopInitProcessingAlg", OutputLevel=DEBUG)
EventLoopFinalProcAlg = IncidentProcAlg(
    "EventLoopFinalProcessingAlg", OutputLevel=DEBUG
)
# add two dummy incident aware services that register
AITestSvc1 = IncidentAsyncTestSvc(
    "IncidentAwareService1",
    FileOffset=10000000,
    EventMultiplier=1000,
    IncidentNames=["BeginEvent", "EndEvent", "BeginRun", "EndRun", "AbortEvent"],
    Priority=1000,
)
AITestSvc2 = IncidentAsyncTestSvc(
    "IncidentAwareService2",
    FileOffset=10000000 * 100,
    EventMultiplier=1000 * 1000,
    IncidentNames=["BeginEvent", "EndEvent"],
    Priority=1,
)
AITestAlg1 = IncidentAsyncTestAlg(
    "IncidentAwareTestAlg1",
    ServiceName="IncidentAwareService1",
    Cardinality=5,
    inpKeys=["/Event/a1"],
)

AITestAlg2 = IncidentAsyncTestAlg(
    "IncidentAwareTestAlg2",
    ServiceName="IncidentAwareService2",
    Cardinality=1,
    inpKeys=["/Event/a2"],
)

FakeInput = CPUCruncher(
    "FakeInput",
    outKeys=["/Event/a1", "/Event/a3", "/Event/a4"],
    varRuntime=0.1,
    avgRuntime=0.1,
)

Producer1 = CPUCruncher(
    "Producer1", inpKeys=["/Event/a1", "/Event/a3"], outKeys=["/Event/a5", "/Event/a6"]
)

Filter = CPUCruncher("Filter", inpKeys=["/Event/a4"])

Producer2 = CPUCruncher("Producer2", inpKeys=["/Event/a3"], outKeys=["/Event/a2"])

Filter2 = CPUCruncher("Filter2", inpKeys=["/Event/a2"])

sequence0 = Gaudi__Sequencer("Sequence0")
sequence0.ModeOR = False
sequence0.ShortCircuit = False  # whether the evaluation is lazy or not!
sequence0.Members += [EventLoopInitProcAlg]
sequencex = Gaudi__Sequencer("SequenceX")
sequencex.ModeOR = False
sequencex.ShortCircuit = False  # whether the evaluation is lazy or not!
sequencex.Members += [EventLoopFinalProcAlg]
sequence1 = Gaudi__Sequencer("Sequence1")
sequence1.Members += [FakeInput, Producer1, Filter, Producer2, AITestAlg1, AITestAlg2]
sequence1.ModeOR = False
sequence1.ShortCircuit = False  # whether the evaluation is lazy or not!
sequence2 = Gaudi__Sequencer("Sequence2")
sequence2.Members += [sequence0, sequence1, Filter2, sequencex]

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel="NONE",
    ExtSvc=[whiteboard, AITestSvc1, AITestSvc2],
    EventLoop=slimeventloopmgr,
    TopAlg=[sequence2],
    MessageSvcType="InertMessageSvc",
)

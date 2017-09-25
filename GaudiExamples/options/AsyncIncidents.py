#!/usr/bin/env gaudirun.py
#
# Copied from ForwardSchedulerControlFlowTest.py to setup asynchronous
# incident service test and example
#
#

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, ForwardSchedulerSvc, CPUCruncher, AlgResourcePool, IncidentProcAlg, IncidentSvc, IncidentAsyncTestSvc, IncidentAsyncTestAlg
from Configurables import GaudiSequencer

msgFmt = "% F%40W%S%4W%s%e%15W%X%7W%R%T %0W%M"
msgSvc = InertMessageSvc("MessageSvc", OutputLevel=INFO)
msgSvc.Format = msgFmt
ApplicationMgr().SvcMapping.append(msgSvc)

IncidentSvc(OutputLevel=DEBUG)

# metaconfig
evtslots = 5
evtMax = 20
cardinality = 10
algosInFlight = 10

whiteboard = HiveWhiteBoard("EventDataSvc",
                            EventSlots=evtslots)

slimeventloopmgr = HiveSlimEventLoopMgr(OutputLevel=INFO)

scheduler = ForwardSchedulerSvc(MaxEventsInFlight=evtslots,
                                MaxAlgosInFlight=algosInFlight,
                                OutputLevel=DEBUG)

AlgResourcePool(OutputLevel=DEBUG)
# Async Incident svc processing algs to be added at the beginning and
# at the end of event graph to process incidents fired in the context
# of the given event

EventLoopInitProcAlg = IncidentProcAlg("EventLoopInitProcessingAlg",
                                       OutputLevel=DEBUG)
EventLoopFinalProcAlg = IncidentProcAlg("EventLoopFinalProcessingAlg",
                                        OutputLevel=DEBUG)
# add two dummy incident aware services that register
AITestSvc1 = IncidentAsyncTestSvc("IncidentAwareService1",
                                  FileOffset=10000000,
                                  EventMultiplier=1000,
                                  IncidentNames=[
                                      'BeginEvent', 'EndEvent', 'BeginRun', 'EndRun', 'AbortEvent'],
                                  Priority=1000)
AITestSvc2 = IncidentAsyncTestSvc("IncidentAwareService2",
                                  FileOffset=10000000 * 100,
                                  EventMultiplier=1000 * 1000,
                                  IncidentNames=['BeginEvent', 'EndEvent'],
                                  Priority=1)
AITestAlg1 = IncidentAsyncTestAlg("IncidentAwareTestAlg1",
                                  ServiceName="IncidentAwareService1",
                                  Cardinality=5,
                                  inpKeys=['/Event/DAQ/ODIN']
                                  )

AITestAlg2 = IncidentAsyncTestAlg("IncidentAwareTestAlg2",
                                  ServiceName="IncidentAwareService2",
                                  Cardinality=1,
                                  inpKeys=['/Event/Hlt/DecReports']
                                  )


FakeInput = CPUCruncher("FakeInput",
                        outKeys=['/Event/DAQ/ODIN', '/Event/DAQ/RawEvent',
                                 '/Event/Hlt/LumiSummary'],
                        shortCalib=True,
                        varRuntime=.1,
                        avgRuntime=.1)

BrunelInit = CPUCruncher("BrunelInit",
                         inpKeys=['/Event/DAQ/ODIN', '/Event/DAQ/RawEvent'],
                         outKeys=['/Event/Rec/Status', '/Event/Rec/Header'],
                         shortCalib=True)

PhysFilter = CPUCruncher("PhysFilter",
                         shortCalib=True,
                         inpKeys=['/Event/Hlt/LumiSummary'])

HltDecReportsDecoder = CPUCruncher("HltDecReportsDecoder",
                                   shortCalib=True,
                                   inpKeys=['/Event/DAQ/RawEvent'],
                                   outKeys=['/Event/Hlt/DecReports'])

HltErrorFilter = CPUCruncher("HltErrorFilter",
                             shortCalib=True,
                             inpKeys=['/Event/Hlt/DecReports'])

sequence0 = GaudiSequencer("Sequence0")
sequence0.ModeOR = False
sequence0.ShortCircuit = False  # whether the evaluation is lazy or not!
sequence0.Members += [EventLoopInitProcAlg]
sequencex = GaudiSequencer("SequenceX")
sequencex.ModeOR = False
sequencex.ShortCircuit = False  # whether the evaluation is lazy or not!
sequencex.Members += [EventLoopFinalProcAlg]
sequence1 = GaudiSequencer("Sequence1")
sequence1.Members += [FakeInput, BrunelInit, PhysFilter,
                      HltDecReportsDecoder, AITestAlg1, AITestAlg2]
sequence1.ModeOR = False
sequence1.ShortCircuit = False  # whether the evaluation is lazy or not!
sequence2 = GaudiSequencer("Sequence2")
sequence2.Members += [sequence0, sequence1, HltErrorFilter, sequencex]


ApplicationMgr(EvtMax=evtMax,
               EvtSel='NONE',
               ExtSvc=[whiteboard, AITestSvc1, AITestSvc2],
               EventLoop=slimeventloopmgr,
               TopAlg=[sequence2],
               MessageSvcType="InertMessageSvc")

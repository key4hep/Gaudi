#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, ForwardSchedulerSvc, CPUCruncher, AlgResourcePool
from Configurables import GaudiSequencer

InertMessageSvc(OutputLevel=INFO)

# metaconfig
evtslots = 1
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

FakeInput = CPUCruncher("FakeInput",
                        DataOutputs=[
                            '/Event/DAQ/ODIN', '/Event/DAQ/RawEvent', '/Event/Hlt/LumiSummary'],
                        shortCalib=True,
                        varRuntime=.1,
                        avgRuntime=.1)

BrunelInit = CPUCruncher("BrunelInit",
                         DataInputs=['/Event/DAQ/ODIN', '/Event/DAQ/RawEvent'],
                         DataOutputs=['/Event/Rec/Status',
                                      '/Event/Rec/Header'],
                         shortCalib=True)

PhysFilter = CPUCruncher("PhysFilter",
                         shortCalib=True,
                         DataInputs=['/Event/Hlt/LumiSummary'])

HltDecReportsDecoder = CPUCruncher("HltDecReportsDecoder",
                                   shortCalib=True,
                                   DataInputs=['/Event/DAQ/RawEvent'],
                                   DataOutputs=['/Event/Hlt/DecReports'])

HltErrorFilter = CPUCruncher("HltErrorFilter",
                             shortCalib=True,
                             DataInputs=['/Event/Hlt/DecReports'])

sequence1 = GaudiSequencer("Sequence1")
sequence1.Members += [FakeInput, BrunelInit, PhysFilter, HltDecReportsDecoder]
sequence1.ModeOR = False
sequence1.ShortCircuit = False  # whether the evaluation is lazy or not!
sequence2 = GaudiSequencer("Sequence2")
sequence2.Members += [sequence1, HltErrorFilter]


ApplicationMgr(EvtMax=evtMax,
               EvtSel='NONE',
               ExtSvc=[whiteboard],
               EventLoop=slimeventloopmgr,
               TopAlg=[sequence2],
               MessageSvcType="InertMessageSvc")

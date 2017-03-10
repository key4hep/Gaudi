#!/usr/bin/env gaudirun.py

'''
Reference optionfile which shows in a simple way how to take advantage of the
Gaudi components desicated to concurrency.
The components are:
 o HiveWhiteBoard: a convenient way to collect several TES (one per "processing
 slot"), accessible in a thread safe way, keeps a catalogue of the products
 written on each processing slot. The number of slots in the whiteboard
 determines also the number of events processed simultaneously by the scheduler.
 o AvalancheSchedulerSvc: state machine of the algorithms interfaced with the
 TBB runtime. It is responsible for the submission of the algorithms. An
 algorithm is marked ready for submission when its needed input is available.
 It deals the asynchronous termination of algorithms with a "receiver" thread
 and a thread safe queue.
 o HiveSlimEventLoopMgr: an event factory. Pushes new events and pops finished
 events to/from the scheduler. It does not manage algorithms/streams.
 o AlgResourcePool: Service managing the creation of algorithms (through the
 algorithm manager), including clones. It also manages the algorithms according
 to the resources they need (parameter NeededResources - vector of strings - of
 the Algorithm class).
 o InertMessageSvc: as the TBBMsgSvc, it manages the printing of the messages in
 a multithreaded environment.

The CPUCruncher is not a component dealing with concurrency, but a useful
entity to test it. It's an algorithm that simply wastes cpu.

'''
from Gaudi.Configuration import *
from Configurables import (HiveWhiteBoard, HiveSlimEventLoopMgr,
                           AvalancheSchedulerSvc, AlgResourcePool,
                           CPUCruncher,
                           ContextEventCounterPtr,
                           ContextEventCounterData)

# metaconfig -------------------------------------------------------------------
# It's confortable to collect the relevant parameters at the top of the optionfile
evtslots = 23
evtMax = 50
cardinality=10
algosInFlight=10
#-------------------------------------------------------------------------------

# The configuration of the whiteboard ------------------------------------------
# It is useful to call it EventDataSvc to replace the usual data service with
# the whiteboard transparently.

whiteboard   = HiveWhiteBoard("EventDataSvc",
                              EventSlots = evtslots)

#-------------------------------------------------------------------------------

# Event Loop Manager -----------------------------------------------------------
# It's called slim since it has less functionalities overall than the good-old
# event loop manager. Here we just set its outputlevel to DEBUG.

slimeventloopmgr = HiveSlimEventLoopMgr(SchedulerName = "AvalancheSchedulerSvc", OutputLevel=DEBUG)

#-------------------------------------------------------------------------------

# ForwardScheduler -------------------------------------------------------------
# We just decide how many algorithms in flight we want to have and how many
# threads in the pool. The default value is -1, which is for TBB equivalent
# to take over the whole machine.

scheduler = AvalancheSchedulerSvc(MaxAlgosInFlight = algosInFlight,
                                  ThreadPoolSize = algosInFlight,
                                  OutputLevel=WARNING)

#-------------------------------------------------------------------------------

# Algo Resource Pool -----------------------------------------------------------
# Nothing special here, we just set the debug level.
AlgResourcePool(OutputLevel=DEBUG)

#-------------------------------------------------------------------------------

# Set up of the crunchers, daily business --------------------------------------

a1 = CPUCruncher("A1")
a1.outKeys = ['/Event/a1']

a2 = CPUCruncher("A2")
a2.inpKeys = ['/Event/a1']
a2.outKeys = ['/Event/a2']

a3 = CPUCruncher("A3")
a3.inpKeys = ['/Event/a1']
a3.outKeys = ['/Event/a3']

a4 = CPUCruncher("A4")
a4.inpKeys = ['/Event/a2','/Event/a3']
a4.outKeys = ['/Event/a4']

for algo in [a1, a2, a3, a4]:
  algo.shortCalib=True
  algo.Cardinality = cardinality
  algo.OutputLevel=WARNING
  algo.varRuntime=.3
  algo.avgRuntime=.5

ctrp = ContextEventCounterPtr("CNT*", Cardinality=0, OutputLevel=INFO)
ctrd = ContextEventCounterData("CNT&", Cardinality=0, OutputLevel=INFO)

# Application Manager ----------------------------------------------------------
# We put everything together and change the type of message service

ApplicationMgr( EvtMax = evtMax,
                EvtSel = 'NONE',
                ExtSvc =[whiteboard],
                EventLoop = slimeventloopmgr,
                TopAlg = [a1, a2, a3, a4, ctrp, ctrd],
                MessageSvcType="InertMessageSvc")

#-------------------------------------------------------------------------------

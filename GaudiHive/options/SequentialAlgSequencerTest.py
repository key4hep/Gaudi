from Gaudi.Configuration import *
from Configurables import (HiveWhiteBoard, HiveSlimEventLoopMgr,
                           AvalancheSchedulerSvc, AlgResourcePool,
                           CPUCruncher,
                           ContextEventCounterPtr,
                           ContextEventCounterData,
                           GaudiSequencer)

# metaconfig -------------------------------------------------------------------
# It's confortable to collect the relevant parameters at the top of the optionfile
evtslots = 5
evtMax = 20
cardinality = 5
algosInFlight = 10
# -------------------------------------------------------------------------------

# The configuration of the whiteboard ------------------------------------------
# It is useful to call it EventDataSvc to replace the usual data service with
# the whiteboard transparently.

whiteboard = HiveWhiteBoard("EventDataSvc",
                            EventSlots=evtslots)

# -------------------------------------------------------------------------------

# Event Loop Manager -----------------------------------------------------------
# It's called slim since it has less functionalities overall than the good-old
# event loop manager. Here we just set its outputlevel to DEBUG.

slimeventloopmgr = HiveSlimEventLoopMgr(OutputLevel=DEBUG)

# -------------------------------------------------------------------------------

# Avalanche Scheduler ----------------------------------------------------------
# We just decide how many algorithms in flight we want to have and how many
# threads in the pool. The default value is -1, which is for TBB equivalent
# to take over the whole machine.

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=algosInFlight,
                                  OutputLevel=DEBUG)

# -------------------------------------------------------------------------------

# Algo Resource Pool -----------------------------------------------------------
# Nothing special here, we just set the debug level.
AlgResourcePool(OutputLevel=DEBUG)

# -------------------------------------------------------------------------------

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
a4.inpKeys = ['/Event/a2']
a4.outKeys = ['/Event/a4']

for algo in [a1, a2, a3, a4]:
    algo.shortCalib = True
    algo.OutputLevel = DEBUG
    algo.varRuntime = .3
    algo.avgRuntime = .5

for algo in [a3]:
    algo.Cardinality = cardinality

seq = GaudiSequencer("CriticalSection",
                     Members=[a1, a2, a4],
                     Sequential=True,
                     OutputLevel=VERBOSE)

# Application Manager ----------------------------------------------------------
# We put everything together and change the type of message service

ApplicationMgr(EvtMax=evtMax,
               EvtSel='NONE',
               ExtSvc=[whiteboard],
               EventLoop=slimeventloopmgr,
               TopAlg=[seq, a3],
               MessageSvcType="InertMessageSvc")

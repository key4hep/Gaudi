#####################################################################################
# (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################

from Configurables import Gaudi__TestSuite__AsyncSleeperAlg as AsyncSleeperAlg
from Configurables import TimelineSvc
from Configurables import Gaudi__Sequencer as Sequencer
from Configurables import (
    AvalancheSchedulerSvc,
    HiveSlimEventLoopMgr,
    HiveWhiteBoard,
)
from Configurables import GaudiTestSuite__LoggingAuditor as LoggingAuditor
from Configurables import AuditorSvc
from Gaudi.Configuration import *


threads = 2
offloadThreads = 2
evtslots = 1

# Configure parallel services
slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=ERROR
)
whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots)
scheduler = AvalancheSchedulerSvc(
    ThreadPoolSize=threads, NumOffloadThreads=offloadThreads, OutputLevel=WARNING
)

# Application setup
app = ApplicationMgr()
# - Algorithms
singleSleepAlg = AsyncSleeperAlg(
    "SingleSleepAlg",
    SuspensionDuration=100,
    SuspensionPoints=1,
    OutputLevel=WARNING,
)  # Single suspension -> two timeline events
doubleSleepAlg = AsyncSleeperAlg(
    "DoubleSleepAlg",
    SuspensionDuration=100,
    SuspensionPoints=2,
    OutputLevel=WARNING,
)  # Two suspensions -> three timeline events
sleepersSequencer = Sequencer(
    "Sleepers",
    Sequential=True,
    Members=[singleSleepAlg, doubleSleepAlg],
)  # No data dependencies between the two algorithms but executed in sequence due to the sequential flag.

app.TopAlg = [sleepersSequencer]
# - Events
app.EvtMax = 2
app.EvtSel = "NONE"
app.HistogramPersistency = "NONE"
app.EventLoop = slimeventloopmgr
app.ExtSvc = [whiteboard]
app.AuditAlgorithms = True
# - Activate timeline service
TimelineSvc(RecordTimeline=True, DumpTimeline=True, TimelineFile="myAsyncTimeline.csv")
# - Activate auditor service
auditor = LoggingAuditor("Auditor")
AuditorSvc(Auditors=[auditor])

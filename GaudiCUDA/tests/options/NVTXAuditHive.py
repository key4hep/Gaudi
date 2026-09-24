#####################################################################################
# (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################

from Configurables import Gaudi__Sequencer as Sequencer
from Configurables import (
    AvalancheSchedulerSvc,
    HiveSlimEventLoopMgr,
    HiveWhiteBoard,
)
from Configurables import NVTXAuditor
from Configurables import CPUCruncher
from Configurables import GPUCruncher
from Configurables import AuditorSvc
from Gaudi.Configuration import *

# Application setup
app = ApplicationMgr()

# Configure parallel services

threads = 1
offloadThreads = 1
evtslots = 2

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=INFO
)
whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots, OutputLevel=ERROR)
scheduler = AvalancheSchedulerSvc(
    ThreadPoolSize=threads, NumOffloadThreads=offloadThreads, OutputLevel=ERROR
)

app.EventLoop = slimeventloopmgr
app.ExtSvc = [whiteboard]

# Auditors
AuditorSvc(Auditors=[NVTXAuditor(OutputLevel=INFO)])
app.AuditAlgorithms = True

# - Algorithms
GPUAlg1 = GPUCruncher("GPUAlg1", avgRuntime=0.01, OutputLevel=ERROR)
GPUAlg2 = GPUCruncher("GPUAlg2", avgRuntime=0.01, OutputLevel=ERROR)
CPUAlg1 = CPUCruncher("CPUAlg1", avgRuntime=0.01, OutputLevel=ERROR)
CPUAlg2 = CPUCruncher("CPUAlg2", avgRuntime=0.01, OutputLevel=ERROR)

app.TopAlg = [GPUAlg1, GPUAlg2, CPUAlg1, CPUAlg2]

# - Events
app.EvtMax = 4
app.EvtSel = "NONE"
app.HistogramPersistency = "NONE"

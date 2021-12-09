#!/usr/bin/env gaudirun.py
#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
"""
Options file to test fix for https://sft.its.cern.ch/jira/browse/CFHEP-114
a problem which is ther when declaring dependencies explicitely in the scheduler
and having more dependencies than algorithms.
"""

from Configurables import (
    AlgResourcePool,
    AvalancheSchedulerSvc,
    CPUCruncher,
    CPUCrunchSvc,
    HiveSlimEventLoopMgr,
    HiveWhiteBoard,
)
from Gaudi.Configuration import *

InertMessageSvc(OutputLevel=INFO)

# metaconfig
evtslots = 13
evtMax = 50
cardinality = 10
algosInFlight = 10

whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots)

slimeventloopmgr = HiveSlimEventLoopMgr(OutputLevel=INFO)

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=algosInFlight, OutputLevel=WARNING)

AlgResourcePool(OutputLevel=DEBUG)

CPUCrunchSvc(shortCalib=True)

a1 = CPUCruncher("A1", varRuntime=0.01, avgRuntime=0.1)
a1.outKeys = ["/Event/a1"]

a2 = CPUCruncher("A2")
a2.outKeys = ["/Event/a2"]

a3 = CPUCruncher("A3")
a3.outKeys = ["/Event/a3", "/Event/a4"]

a4 = CPUCruncher("A4")
a4.outKeys = ["/Event/a5"]

for algo in [a1, a2, a3, a4]:
    algo.Cardinality = cardinality
    algo.OutputLevel = WARNING

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel="NONE",
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[a1, a2, a3, a4],
    MessageSvcType="InertMessageSvc",
)

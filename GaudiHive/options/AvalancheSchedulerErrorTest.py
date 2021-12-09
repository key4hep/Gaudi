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
Test the correct handling on errors during the event processing:
- The CPUCruncher is configured to return an ERROR on a fraction of the events
- The EventLoopMgr is configured to continue with the next event on ERROR
"""

from Configurables import (
    AlgResourcePool,
    ApplicationMgr,
    AvalancheSchedulerSvc,
    CPUCruncher,
    CPUCrunchSvc,
    HiveSlimEventLoopMgr,
    HiveWhiteBoard,
    InertMessageSvc,
)
from Gaudi.Configuration import *

evtslots = 8
evtMax = 50
cardinality = 10
threads = 10

whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots)

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=DEBUG, AbortOnFailure=False
)

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=threads, OutputLevel=DEBUG)

AlgResourcePool(OutputLevel=DEBUG)
CPUCrunchSvc(shortCalib=True)

#
# - A3 returns an ERROR once in a while
# - A1-A2-A4 execute normally and then a stall is detected
#
#      A1
#     /  \
#    A2  A3
#    |   |
#    A4  A5
#
a1 = CPUCruncher("A1")
a1.outKeys = ["/Event/a1"]

a2 = CPUCruncher("A2")
a2.inpKeys = ["/Event/a1"]
a2.outKeys = ["/Event/a2"]

a3 = CPUCruncher("A3")
a3.inpKeys = ["/Event/a1"]
a3.outKeys = ["/Event/a3"]
a3.FailNEvents = 7  # produce ERROR every Nth event

a4 = CPUCruncher("A4")
a4.inpKeys = ["/Event/a2"]
a4.outKeys = ["/Event/a4"]

a5 = CPUCruncher("A5")
a5.inpKeys = ["/Event/a3"]
a5.outKeys = ["/Event/a5"]

for algo in [a1, a2, a3, a4, a5]:
    algo.Cardinality = cardinality
    algo.avgRuntime = 0.1

msgSvc = InertMessageSvc("MessageSvc", Format="% F%30W%S%4W%e%s%7W%R%T %0W%M")
ApplicationMgr().SvcMapping.append(msgSvc)

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel="NONE",
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[a1, a2, a3, a4, a5],
    MessageSvcType="InertMessageSvc",
)

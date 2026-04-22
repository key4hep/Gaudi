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

from Configurables import (
    AlgResourcePool,
    AvalancheSchedulerSvc,
    HiveSlimEventLoopMgr,
    HiveWhiteBoard,
    ReEntAlg,
)
from Gaudi.Configuration import ApplicationMgr, WARNING

nSlots = 4
nThread = 4

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=nThread)
slimeventloopmgr = HiveSlimEventLoopMgr(SchedulerName=scheduler)
whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=nSlots)

# Configure available and needed resources. Even though enough threads/slots
# are available, the algorithm should only be executed concurrently up to the
# available number of "Tokens".
algResourcePool = AlgResourcePool(
    AvailableResources={"Tokens": 2, "Dummy": 100}, MissingResourceMessageLevel=WARNING
)

myralg = ReEntAlg("ReEntAlg", SleepFor=1000, NeededResources={"Tokens": 1, "Dummy": 5})

ApplicationMgr(
    EvtMax=5,
    EvtSel="NONE",
    HistogramPersistency="NONE",
    EventLoop=slimeventloopmgr,
    ExtSvc=[algResourcePool, whiteboard],
    TopAlg=[myralg],
    MessageSvcType="InertMessageSvc",
)

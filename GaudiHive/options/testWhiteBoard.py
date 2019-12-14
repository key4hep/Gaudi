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
from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, HiveTestAlgorithm, AvalancheSchedulerSvc

evtslots = 10

whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots)

slimeventloopmgr = HiveSlimEventLoopMgr(OutputLevel=DEBUG)

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=8, OutputLevel=WARNING)

a1 = HiveTestAlgorithm("A1", Output=['/Event/a1'])
a2 = HiveTestAlgorithm("A2", Input=['/Event/a1'], Output=['/Event/a2'])
a3 = HiveTestAlgorithm("A3", Input=['/Event/a1'], Output=['/Event/a3'])
a4 = HiveTestAlgorithm(
    "A4", Input=['/Event/a2', '/Event/a3'], Output=['/Event/a4'])

ApplicationMgr(
    EvtMax=100,
    EvtSel='NONE',
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[a1, a2, a3, a4],
    MessageSvcType="InertMessageSvc")

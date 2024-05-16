#####################################################################################
# (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
###############################################################
# Job options file
# ==============================================================
from Configurables import ApplicationMgr, AvalancheSchedulerSvc
from Configurables import Gaudi__TestSuite__IntDataConsumer as IntDataConsumer
from Configurables import GaudiExamplesCommonConf, HiveSlimEventLoopMgr, HiveWhiteBoard
from Gaudi.Configuration import ERROR, WARNING

threads = 1

GaudiExamplesCommonConf()

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=ERROR
)
whiteboard = HiveWhiteBoard("EventDataSvc")

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=threads, OutputLevel=WARNING)

ApplicationMgr(
    TopAlg=[IntDataConsumer()],
    EvtMax=10,
    EvtSel="NONE",
    EventLoop=slimeventloopmgr,
    ExtSvc=[whiteboard],
)

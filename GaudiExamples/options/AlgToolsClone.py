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
###############################################################
# Job options file
# ==============================================================
from Gaudi.Configuration import *
from Configurables import MyAlgorithm, MyGaudiAlgorithm, ToolSvc, AlgResourcePool, HiveSlimEventLoopMgr, HiveWhiteBoard

from Configurables import GaudiExamplesCommonConf
GaudiExamplesCommonConf()

myalg = MyAlgorithm('MyAlg', PrivateToolsOnly=True, Cardinality=20)

ToolSvc(OutputLevel=INFO)

algResourcePool = AlgResourcePool(OutputLevel=INFO)
slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=INFO)
whiteboard = HiveWhiteBoard("EventDataSvc")

ApplicationMgr(
    EvtMax=1,
    EvtSel='NONE',
    HistogramPersistency='NONE',
    EventLoop=slimeventloopmgr,
    ExtSvc=[algResourcePool, whiteboard],
    TopAlg=[myalg])

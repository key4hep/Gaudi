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
from Configurables import ReEntAlg

from Configurables import AlgResourcePool, HiveSlimEventLoopMgr, HiveWhiteBoard, AvalancheSchedulerSvc
# from Configurables import GaudiExamplesCommonConf
# GaudiExamplesCommonConf()

nSlots = 3
nThread = 3

algCard = 0

# msgFmt = "% F%40W%S%4W%s%e%7W%R%T %0W%M"
# msgSvc = MessageSvc()
# msgSvc.Format = msgFmt

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=nThread)
algResourcePool = AlgResourcePool()
slimeventloopmgr = HiveSlimEventLoopMgr(SchedulerName=scheduler)
whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=nSlots)

myralg = ReEntAlg(
    'ReEntAlg', OutputLevel=INFO, Cardinality=algCard, SleepFor=2000)

ApplicationMgr(
    EvtMax=10,
    EvtSel='NONE',
    HistogramPersistency='NONE',
    EventLoop=slimeventloopmgr,
    ExtSvc=[algResourcePool, whiteboard],
    TopAlg=[myralg],
    OutputLevel=INFO,
    MessageSvcType="InertMessageSvc")

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
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, AvalancheSchedulerSvc
from Configurables import Gaudi__Examples__THDataProducer as THDataProducer
from Configurables import Gaudi__Examples__THDataProducer2 as THDataProducer2
from Configurables import Gaudi__Examples__THDataConsumer as THDataConsumer

# Application setup
whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=2)
slimeventloopmgr = HiveSlimEventLoopMgr(OutputLevel=INFO)
scheduler = AvalancheSchedulerSvc(ThreadPoolSize=2)

# - Algorithms
topalgs = [
    THDataProducer("THDataProducer"),
    THDataProducer2("THDataProducer2"),
    THDataConsumer("THDataConsumer")
]

# Application manager
app = ApplicationMgr(
    EvtMax=4, ExtSvc=[whiteboard], EventLoop=slimeventloopmgr, TopAlg=topalgs)

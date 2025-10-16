#####################################################################################
# (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
####################################################################
# Write a DST and a miniDST, including File Summary Records
####################################################################

from Configurables import Gaudi__RootCnvSvc as RootCnvSvc
from Configurables import (
    AlgResourcePool,
    GaudiPersistency,
    HiveSlimEventLoopMgr,
    HiveWhiteBoard,
    ReadAlg,
)
from Gaudi.Configuration import *

# I/O
GaudiPersistency()
FileCatalog(Catalogs=["xmlcatalog_file:ROOTIO.xml"])
esel = EventSelector(OutputLevel=DEBUG, PrintFreq=50, FirstEvent=1)
esel.Input = [
    "DATAFILE='PFN:ROOTIO.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'",
    "DATAFILE='PFN:ROOTIO.mdst' SVC='Gaudi::RootEvtSelector' OPT='READ'",
]

readAlg = ReadAlg(OutputLevel=VERBOSE)

algResourcePool = AlgResourcePool(OutputLevel=INFO)
slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=INFO
)
whiteboard = HiveWhiteBoard("EventDataSvc")
app = ApplicationMgr(
    EvtMax=2000,
    HistogramPersistency="NONE",
    EventLoop=slimeventloopmgr,
    ExtSvc=[algResourcePool, whiteboard],
    TopAlg=[readAlg],
)
RootCnvSvc(OutputLevel=INFO)

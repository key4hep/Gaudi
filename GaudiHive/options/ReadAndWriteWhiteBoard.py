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
####################################################################
# Write a DST and a miniDST, including File Summary Records
####################################################################

from Configurables import AvalancheSchedulerSvc
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc
from Configurables import (
    GaudiPersistency,
    HiveReadAlgorithm,
    HiveSlimEventLoopMgr,
    HiveWhiteBoard,
    ReadHandleAlg,
)
from Gaudi.Configuration import *

# Output Levels
MessageSvc(OutputLevel=WARNING)
IncidentSvc(OutputLevel=DEBUG)
RootCnvSvc(OutputLevel=INFO)

# Output setup
# - DST
dst = OutputStream("RootDst")
dst.ItemList = ["/Event#999"]
dst.Output = (
    "DATAFILE='PFN:HandleWB_ROOTOutput.dst'  SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"
)
dst.NeededResources = ["ROOTIO"]

GaudiPersistency()

EventSelector(
    OutputLevel=DEBUG,
    PrintFreq=50,
    FirstEvent=1,
    Input=[
        "DATAFILE='PFN:HandleWB_ROOTIO.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'"
    ],
)
FileCatalog(Catalogs=["xmlcatalog_file:HandleWB_ROOTIO.xml"])

product_name = "MyCollision"
product_name_full_path = "/Event/" + product_name

loader = HiveReadAlgorithm("Loader", OutputLevel=INFO, NeededResources=["ROOTIO"])

reader = ReadHandleAlg("Reader", OutputLevel=INFO, NeededResources=["ROOTIO"])
reader.Input.Path = product_name

evtslots = 6
algoparallel = 20

whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots)

eventloopmgr = HiveSlimEventLoopMgr(OutputLevel=INFO)

# We must put the full path in this deprecated expression of dependencies.
# Using a controlflow for the output would be the way to go
scheduler = AvalancheSchedulerSvc(
    ThreadPoolSize=algoparallel,
    OutputLevel=WARNING,
    CheckDependencies=True,
    DataLoaderAlg=loader.name(),
)

ApplicationMgr(
    TopAlg=[loader, reader, dst],
    EvtMax=44,
    HistogramPersistency="NONE",
    ExtSvc=[whiteboard],
    EventLoop=eventloopmgr,
)

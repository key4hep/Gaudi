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

from Gaudi.Configuration import *
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc, GaudiPersistency, StoreSnifferAlg
from Configurables import WriteHandleAlg, ReadHandleAlg, HiveWhiteBoard, HiveSlimEventLoopMgr, HiveReadAlgorithm, AvalancheSchedulerSvc, AlgResourcePool

# Output Levels
MessageSvc(OutputLevel=WARNING)
IncidentSvc(OutputLevel=INFO)
RootCnvSvc(OutputLevel=INFO)
SequencerTimerTool(OutputLevel=WARNING)

GaudiPersistency()

EventSelector(
    OutputLevel=DEBUG,
    PrintFreq=50,
    FirstEvent=1,
    Input=[
        "DATAFILE='PFN:HandleWB_ROOTIO.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'"
    ])
FileCatalog(Catalogs=["xmlcatalog_file:HandleWB_ROOTIO.xml"])

product_name = "MyCollision"
product_name_full_path = "/Event/" + product_name

loader = HiveReadAlgorithm(
    "Loader",
    OutputLevel=INFO,
    NeededResources=['ROOTIO', 'SOMETHINGELSE'],
    Cardinality=2  # framework should be able to fix this config problem
)

sniffer = StoreSnifferAlg()
reader = ReadHandleAlg("Reader", Cardinality=4, OutputLevel=INFO)
reader.Input.Path = product_name

evtslots = 1
algoparallel = 10

whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots)

eventloopmgr = HiveSlimEventLoopMgr(OutputLevel=INFO)

# We must put the full path in this deprecated expression of dependencies.
# Using a controlflow for the output would be the way to go
scheduler = AvalancheSchedulerSvc(
    ThreadPoolSize=algoparallel,
    OutputLevel=WARNING,
    DataLoaderAlg=loader.name(),
    CheckDependencies=True)

# Application setup
ApplicationMgr(
    TopAlg=[loader, sniffer, reader],
    EvtMax=500,
    HistogramPersistency="NONE",
    ExtSvc=[whiteboard],
    EventLoop=eventloopmgr)

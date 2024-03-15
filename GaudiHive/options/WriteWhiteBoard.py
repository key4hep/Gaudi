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
####################################################################
# Write a DST and a miniDST, including File Summary Records
####################################################################

from Configurables import AlgResourcePool, AvalancheSchedulerSvc
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc
from Configurables import Gaudi__TestSuite__WriteHandleAlg as WriteHandleAlg
from Configurables import GaudiPersistency, HiveSlimEventLoopMgr, HiveWhiteBoard
from Gaudi.Configuration import *

# Output setup
# - DST
dst = OutputStream("RootDst")

dst.ItemList = ["/Event#999"]
dst.Output = (
    "DATAFILE='PFN:HandleWB_ROOTIO.dst'  SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"
)

# - MiniDST
mini = OutputStream("RootMini")
mini.ItemList = ["/Event#1"]
mini.Output = (
    "DATAFILE='PFN:HandleWB_ROOTIO.mdst' SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"
)
mini.OutputLevel = VERBOSE

# - File Summary Record
fsr = RecordStream("FileRecords")
fsr.ItemList = ["/FileRecords#999"]
fsr.Output = dst.Output
fsr.EvtDataSvc = FileRecordDataSvc()
fsr.EvtConversionSvc = FileRecordPersistencySvc()

FileCatalog(Catalogs=["xmlcatalog_file:HandleWB_ROOTIO.xml"])

# Output Levels
MessageSvc(OutputLevel=WARNING)
IncidentSvc(OutputLevel=DEBUG)
RootCnvSvc(OutputLevel=INFO)
AlgResourcePool(OutputLevel=DEBUG)
GaudiPersistency()

product_name = "MyCollision"
product_name_full_path = "/Event/" + product_name

writer = WriteHandleAlg("Writer", UseHandle=True, Cardinality=1, OutputLevel=WARNING)

writer.Output.Path = "/Event/" + product_name

evtslots = 1
algoparallel = 1

whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots)

eventloopmgr = HiveSlimEventLoopMgr(OutputLevel=INFO)

# We must put the full path in this deprecated expression of dependencies.
# Using a controlflow for the output would be the way to go
scheduler = AvalancheSchedulerSvc(ThreadPoolSize=algoparallel, OutputLevel=INFO)

# Application setup
ApplicationMgr(
    TopAlg=[writer, dst],
    EvtMax=500,
    EvtSel="NONE",  # do not use any event input
    HistogramPersistency="NONE",
    ExtSvc=[whiteboard],
    EventLoop=eventloopmgr,
)

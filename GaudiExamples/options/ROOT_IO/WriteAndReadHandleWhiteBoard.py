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
    HiveSlimEventLoopMgr,
    HiveWhiteBoard,
    ReadHandleAlg,
    WriteHandleAlg,
)
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

GaudiPersistency()

product_name = "/Event/MyCollision"

writer = WriteHandleAlg("Writer", OutputLevel=DEBUG, UseHandle=True)
writer.Output.Path = product_name

reader = ReadHandleAlg("Reader", OutputLevel=DEBUG)
reader.Input.Path = product_name

evtslots = 15
algoparallel = 10

whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots)

slimeventloopmgr = HiveSlimEventLoopMgr()

scheduler = AvalancheSchedulerSvc(ThreadPoolSize=algoparallel, OutputLevel=WARNING)

# Application setup
app = ApplicationMgr()
# - I/O
# Do not put them here, but as normal algorithms.
# Putting two of them in the top alg list will not work if more than one
# algo is allowed to be in flight: at some point they will write to disk
# simultaneously, with catastrophic effects.

# app.OutStream += [ mini ]
# - Algorithms
app.TopAlg = [writer, reader, mini]
# - Events
app.EvtMax = 50
app.EvtSel = "NONE"  # do not use any event input
app.HistogramPersistency = "NONE"
app.ExtSvc = [whiteboard]
app.EventLoop = slimeventloopmgr

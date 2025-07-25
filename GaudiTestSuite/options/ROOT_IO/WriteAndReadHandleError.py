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

from Configurables import Gaudi__RootCnvSvc as RootCnvSvc
from Configurables import Gaudi__TestSuite__ReadHandleAlg as ReadHandleAlg
from Configurables import Gaudi__TestSuite__WriteHandleAlg as WriteHandleAlg
from Configurables import GaudiPersistency
from Gaudi.Configuration import *

# Output setup
# - DST
dst = OutputStream("RootDst")

dst.ItemList = ["/Event#999"]
dst.Output = (
    "DATAFILE='PFN:Handle_ROOTIO_err.dst'  SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"
)

# - MiniDST
mini = OutputStream("RootMini")
mini.ItemList = ["/Event#1"]
mini.Output = (
    "DATAFILE='PFN:Handle_ROOTIO_err.mdst' SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"
)

# - File Summary Record
fsr = RecordStream("FileRecords")
fsr.ItemList = ["/FileRecords#999"]
fsr.Output = dst.Output
fsr.EvtDataSvc = FileRecordDataSvc()
fsr.EvtConversionSvc = FileRecordPersistencySvc()

FileCatalog(Catalogs=["xmlcatalog_file:Handle_ROOTIO_err.xml"])

# Output Levels
MessageSvc(OutputLevel=INFO)
IncidentSvc(OutputLevel=INFO)
RootCnvSvc(OutputLevel=INFO)

GaudiPersistency()

writer = WriteHandleAlg("Writer", UseHandle=True, OutputLevel=DEBUG)
writer.Output.Path = "/Event/MyCollision"

reader = ReadHandleAlg("Reader", OutputLevel=DEBUG)
reader.Input.Path = "MyCollisionXOPSX"  # <-- Mistake

# Application setup
app = ApplicationMgr()
# - I/O
app.OutStream += [dst, mini, fsr]
# - Algorithms
app.TopAlg = [writer, reader]
# - Events
app.EvtMax = 10
app.EvtSel = "NONE"  # do not use any event input
app.HistogramPersistency = "NONE"

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
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc, GaudiPersistency
from Configurables import WriteHandleAlg, ReadHandleAlg

# Output setup
# - DST
dst = OutputStream("RootDst")

dst.ItemList = ["/Event#999"]
dst.Output = "DATAFILE='PFN:Handle_ROOTIO.dst'  SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"

# - MiniDST
mini = OutputStream("RootMini")
mini.ItemList = ["/Event#1"]
mini.Output = "DATAFILE='PFN:Handle_ROOTIO.mdst' SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"

# - File Summary Record
fsr = RecordStream("FileRecords")
fsr.ItemList = ["/FileRecords#999"]
fsr.Output = dst.Output
fsr.EvtDataSvc = FileRecordDataSvc()
fsr.EvtConversionSvc = FileRecordPersistencySvc()

FileCatalog(Catalogs=["xmlcatalog_file:Handle_ROOTIO.xml"])

# Output Levels
MessageSvc(OutputLevel=INFO)
IncidentSvc(OutputLevel=INFO)
RootCnvSvc(OutputLevel=INFO)

GaudiPersistency()

writer = WriteHandleAlg("Writer", UseHandle=True, OutputLevel=DEBUG)
writer.DataOutputs.Output.Path = "/Event/MyCollision"

reader = ReadHandleAlg("Reader", OutputLevel=DEBUG)
reader.DataInputs.Input.Path = "MyCollisionXOPSX",  # <-- Mistake

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

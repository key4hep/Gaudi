####################################################################
# Write a DST and a miniDST, including File Summary Records
####################################################################

from Gaudi.Configuration import *
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc, GaudiPersistency

# Output setup
# - DST
dst           = OutputStream("RootDst")
# dst.ItemList  = ["/Event#1",
#                  "/Event/Collision_0#999",
#                  "/Event/Collision_2#1",
#                  "/Event/MyTracks#1",
#                 ]
dst.ItemList  = ["/Event#999"]
dst.Output    = "DATAFILE='PFN:ROOTIO.dst'  SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"

# - MiniDST
mini          = OutputStream("RootMini")
mini.ItemList = ["/Event#1"]
mini.Output   = "DATAFILE='PFN:ROOTIO.mdst' SVC='Gaudi::RootCnvSvc' OPT='RECREATE'";

# - File Summary Record
fsr                   = RecordStream("FileRecords")
fsr.ItemList          = [ "/FileRecords#999" ]
fsr.Output            = dst.Output
fsr.EvtDataSvc        = FileRecordDataSvc()
fsr.EvtConversionSvc  = FileRecordPersistencySvc()

FileCatalog(Catalogs = [ "xmlcatalog_file:ROOTIO.xml" ])

# Output Levels
MessageSvc(OutputLevel=VERBOSE)
IncidentSvc(OutputLevel=DEBUG)
RootCnvSvc(OutputLevel=INFO)

GaudiPersistency()

# Application setup
app = ApplicationMgr()
# - I/O
app.OutStream += [ dst, mini, fsr ]
# - Algorithms
app.TopAlg = [ "WriteAlg", "ReadAlg" ]
# - Events
app.EvtMax   = 1000
app.EvtSel   = "NONE" # do not use any event input
app.HistogramPersistency = "NONE"

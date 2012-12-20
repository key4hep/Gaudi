####################################################################
# Write a DST and a miniDST, including File Summary Records
####################################################################

from Gaudi.Configuration import *
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc, GaudiPersistency
from Configurables import WriteHandleAlg, ReadHandleAlg, HiveWhiteBoard, HiveEventLoopMgr

# Output setup
# - DST
dst           = OutputStream("RootDst")

dst.ItemList  = ["/Event#999"]
dst.Output    = "DATAFILE='PFN:HandleWB_ROOTIO.dst'  SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"

# - MiniDST
mini          = OutputStream("RootMini")
mini.ItemList = ["/Event#1"]
mini.Output   = "DATAFILE='PFN:HandleWB_ROOTIO.mdst' SVC='Gaudi::RootCnvSvc' OPT='RECREATE'";

# - File Summary Record
fsr                   = RecordStream("FileRecords")
fsr.ItemList          = [ "/FileRecords#999" ]
fsr.Output            = dst.Output
fsr.EvtDataSvc        = FileRecordDataSvc()
fsr.EvtConversionSvc  = FileRecordPersistencySvc()

FileCatalog(Catalogs = [ "xmlcatalog_file:HandleWB_ROOTIO.xml" ])

# Output Levels
MessageSvc(OutputLevel=VERBOSE)
IncidentSvc(OutputLevel=DEBUG)
RootCnvSvc(OutputLevel=INFO)

GaudiPersistency()

product_name="MyCollision"

writer = WriteHandleAlg ("Writer",
                         Output="/Event/"+product_name,
                         UseHandle=True)
                         
reader = ReadHandleAlg ("Reader",
                         Input=product_name)                         

                         
evtslots = 10   

whiteboard   = HiveWhiteBoard("EventDataSvc",
                              EventSlots = evtslots)

eventloopmgr = HiveEventLoopMgr(MaxEventsParallel = evtslots,
                                MaxAlgosParallel  = 20,
                                CloneAlgorithms = True,
				DumpQueues = True,
                                AlgosDependencies = [[],[product_name]])
                              
# Application setup
app = ApplicationMgr()
# - I/O for the time being absent because of error to be solved
#app.OutStream += [ dst, mini, fsr ]
# - Algorithms
app.TopAlg = [ writer, reader ]
# - Events
app.EvtMax   = 100
app.EvtSel   = "NONE" # do not use any event input
app.HistogramPersistency = "NONE"
app.ExtSvc = [whiteboard]
app.EventLoop = eventloopmgr

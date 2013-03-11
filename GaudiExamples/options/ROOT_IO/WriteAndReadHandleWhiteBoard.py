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
mini.OutputLevel = VERBOSE


# - File Summary Record
fsr                   = RecordStream("FileRecords")
fsr.ItemList          = [ "/FileRecords#999" ]
fsr.Output            = dst.Output
fsr.EvtDataSvc        = FileRecordDataSvc()
fsr.EvtConversionSvc  = FileRecordPersistencySvc()

FileCatalog(Catalogs = [ "xmlcatalog_file:HandleWB_ROOTIO.xml" ])

# Output Levels
MessageSvc(OutputLevel=WARNING)
IncidentSvc(OutputLevel=DEBUG)
RootCnvSvc(OutputLevel=INFO)

GaudiPersistency()

product_name="MyCollision"

writer = WriteHandleAlg ("Writer",
                         Output="/Event/"+product_name,
                         OutputLevel=DEBUG,
                         UseHandle=True,
                         IsClonable=True)
                         
reader = ReadHandleAlg ("Reader",
                         Input=product_name,
                         OutputLevel=DEBUG,
                         IsClonable=True)                         

                         
evtslots = 15
algoparallel = 10

whiteboard   = HiveWhiteBoard("EventDataSvc",
                              EventSlots = evtslots)
                                                                                     
eventloopmgr = HiveEventLoopMgr(MaxEventsParallel = evtslots,
                                MaxAlgosParallel  = algoparallel,
                                CloneAlgorithms = True,
                                DumpQueues = True,
                                NumThreads = algoparallel,
                                AlgosDependencies = [[],[product_name],[product_name]])

                                
# Application setup
app = ApplicationMgr()
# - I/O
# Do not put them here, but as normal algorithms.
# Putting two of them in the top alg list will not work if more than one
# algo is allowed to be in flight: at some point they will write to disk 
# simultaneously, with catastrophic effects.

#app.OutStream += [ mini ]
# - Algorithms
app.TopAlg = [ writer, reader,mini]
# - Events
app.EvtMax   = 50
app.EvtSel   = "NONE" # do not use any event input
app.HistogramPersistency = "NONE"
app.ExtSvc = [whiteboard]
app.EventLoop = eventloopmgr

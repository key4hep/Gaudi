####################################################################
# Write a DST and a miniDST, including File Summary Records
####################################################################

from Gaudi.Configuration import *
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc, GaudiPersistency
from Configurables import WriteHandleAlg, ReadHandleAlg, HiveWhiteBoard, HiveSlimEventLoopMgr, HiveReadAlgorithm, ForwardSchedulerSvc, AlgResourcePool

# Output Levels
MessageSvc(OutputLevel=WARNING)
IncidentSvc(OutputLevel=DEBUG)
RootCnvSvc(OutputLevel=INFO)
SequencerTimerTool(OutputLevel=WARNING)

# Output setup
# - DST
dst = OutputStream("RootDst")
dst.ItemList = ["/Event#999"]
dst.Output = "DATAFILE='PFN:HandleWB_ROOTOutput.dst'  SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"
dst.NeededResources = ['ROOTIO']

GaudiPersistency()

EventSelector(OutputLevel=DEBUG, PrintFreq=50, FirstEvent=1,
              Input=["DATAFILE='PFN:HandleWB_ROOTIO.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'"])
FileCatalog(Catalogs=["xmlcatalog_file:HandleWB_ROOTIO.xml"])

product_name = "MyCollision"
product_name_full_path = "/Event/" + product_name

loader = HiveReadAlgorithm("Loader",
                           OutputLevel=INFO,
                           NeededResources=['ROOTIO']
                           )

reader = ReadHandleAlg("Reader",
                       OutputLevel=INFO,
                       NeededResources=["ROOTIO"])
reader.Input.Path = product_name


evtslots = 6
algoparallel = 20

whiteboard = HiveWhiteBoard("EventDataSvc",
                            EventSlots=evtslots)

eventloopmgr = HiveSlimEventLoopMgr(OutputLevel=INFO)

# We must put the full path in this deprecated expression of dependencies.
# Using a controlflow for the output would be the way to go
scheduler = ForwardSchedulerSvc(MaxAlgosInFlight=algoparallel,
                                OutputLevel=WARNING)

ApplicationMgr(TopAlg=[loader, reader, dst],
               EvtMax=44,
               HistogramPersistency="NONE",
               ExtSvc=[whiteboard],
               EventLoop=eventloopmgr)

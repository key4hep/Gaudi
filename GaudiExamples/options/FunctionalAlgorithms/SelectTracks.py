####################################################################
# Write a DST and a miniDST, including File Summary Records
####################################################################

from Gaudi.Configuration import *
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc, GaudiPersistency
from Configurables import ReadAlg, ReadTES, FileRecordDataSvc
from Configurables import Gaudi__Examples__SelectTracks as SelectTracks
from Configurables import Gaudi__Examples__CountSelectedTracks as CountSelectedTracks

# I/O
GaudiPersistency()
FileRecordDataSvc(IncidentName="NEW_FILE_RECORD")
FileCatalog(Catalogs=["xmlcatalog_file:ROOTIO.xml"])
esel = EventSelector(OutputLevel=DEBUG, PrintFreq=50, FirstEvent=1)
esel.Input = [
    "DATAFILE='PFN:ROOTIO.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'",
    "DATAFILE='PFN:ROOTIO.mdst' SVC='Gaudi::RootEvtSelector' OPT='READ'"
]

# Algorithms
evtAlgs = GaudiSequencer(
    "EventAlgs",
    Members=[SelectTracks(), CountSelectedTracks()],
    VetoObjects=["FSR"])
fsrAlgs = GaudiSequencer(
    "FSRAlgs", Members=[ReadTES(Locations=["FSR"])], RequireObjects=["FSR"])

# Application setup
app = ApplicationMgr()
#app.ExtSvc = [ EvtStoreSvc("EventDataSvc",OutputLevel=DEBUG ) ]
app.ExtSvc = [ EvtStoreSvc("EventDataSvc" ) ]
# - Algorithms
app.TopAlg = [evtAlgs, fsrAlgs]
# - Events
app.EvtMax = -1
app.HistogramPersistency = "NONE"

RootCnvSvc(OutputLevel=INFO)
# ChronoStatSvc(OutputLevel=WARNING)
SequencerTimerTool(OutputLevel=WARNING)

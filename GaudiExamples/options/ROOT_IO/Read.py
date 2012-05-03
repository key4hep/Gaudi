####################################################################
# Write a DST and a miniDST, including File Summary Records
####################################################################

from Gaudi.Configuration import *
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc, GaudiPersistency
from Configurables import ReadAlg, ReadTES, FileRecordDataSvc

# I/O
GaudiPersistency()
FileRecordDataSvc(IncidentName="NEW_FILE_RECORD")
FileCatalog(Catalogs = [ "xmlcatalog_file:ROOTIO.xml" ])
esel = EventSelector(OutputLevel=DEBUG, PrintFreq=50, FirstEvent=1)
esel.Input = ["DATAFILE='PFN:ROOTIO.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'",
              "DATAFILE='PFN:ROOTIO.mdst' SVC='Gaudi::RootEvtSelector' OPT='READ'"]

# Algorithms
evtAlgs = GaudiSequencer("EventAlgs",
                         Members=[ReadAlg(OutputLevel=VERBOSE,
                                          IncidentName=FileRecordDataSvc().IncidentName)],
                         VetoObjects=["FSR"])
fsrAlgs = GaudiSequencer("FSRAlgs",
                         Members=[ReadTES(Locations=["FSR"])],
                         RequireObjects=["FSR"])

# Application setup
app = ApplicationMgr()
# - Algorithms
app.TopAlg = [ evtAlgs, fsrAlgs ]
# - Events
app.EvtMax   = -1
app.HistogramPersistency = "NONE"

RootCnvSvc(OutputLevel=INFO)
#ChronoStatSvc(OutputLevel=WARNING)
SequencerTimerTool(OutputLevel=WARNING)

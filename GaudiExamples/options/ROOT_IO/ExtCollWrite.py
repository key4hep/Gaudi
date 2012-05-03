from Gaudi.Configuration import *
from Configurables import GaudiPersistency, Gaudi__Examples__ExtendedEvtCol as ExtendedEvtCol

# persistency setup
GaudiPersistency()

FileCatalog(Catalogs = [ "xmlcatalog_file:ROOTIO.xml" ])

# Input
esel = EventSelector(PrintFreq=100)
esel.Input = ["DATAFILE='PFN:ROOTIO.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'",
              "DATAFILE='PFN:ROOTIO.mdst' SVC='Gaudi::RootEvtSelector' OPT='READ'"]

# Output
evtColl = TagCollectionSvc("EvtTupleSvc")
evtColl.Output = ["EXTEVT DATAFILE='PFN:ROOT_IO.etags' OPT='RECREATE' SVC='Gaudi::RootCnvSvc'"]

# Algorithms
algs = GaudiSequencer("EventAlgs",
                      Members=[ExtendedEvtCol("Fill",
                                              EvtColLUN="EXTEVT")], # Logical unit for Event Tag Collection
                      VetoObjects=["FSR"])

# Application
app = ApplicationMgr(TopAlg=[algs],
                     EvtMax=-1,
                     HistogramPersistency="NONE")
app.ExtSvc.append(evtColl)

# Verbosity
SequencerTimerTool(OutputLevel=WARNING)

from Gaudi.Configuration import *
from Configurables import (SequencerTimerTool, GaudiPersistency,
                           EvtCollectionWrite)


# Basic configuration for Gaudi persistency
GaudiPersistency()

FileCatalog(Catalogs = [ "xmlcatalog_file:ROOTIO.xml" ])

# Input
esel = EventSelector(PrintFreq=100)
esel.Input = ["DATAFILE='PFN:ROOTIO.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'",
              "DATAFILE='PFN:ROOTIO.mdst' SVC='Gaudi::RootEvtSelector' OPT='READ'"]

# Output
evtColl = TagCollectionSvc("EvtTupleSvc")
evtColl.Output = ["EvtColl DATAFILE='PFN:ROOT_IO.tags' OPT='RECREATE' SVC='Gaudi::RootCnvSvc'"]

# Algorithms
algs = GaudiSequencer("EventAlgs",
                      Members=[EvtCollectionWrite("Writer")],
                      VetoObjects=["FSR"])

# Application
app = ApplicationMgr(TopAlg=[algs],
                     EvtMax=-1,
                     HistogramPersistency="NONE")
app.ExtSvc.append(evtColl)

# Verbosity
SequencerTimerTool(OutputLevel=WARNING)

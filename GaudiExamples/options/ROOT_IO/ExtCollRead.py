from Gaudi.Configuration import *
from Configurables import (SequencerTimerTool, GaudiPersistency, ReadAlg)

# Basic configuration for Gaudi persistency
GaudiPersistency()

FileCatalog(Catalogs = [ "xmlcatalog_file:ROOTIO.xml" ])

# Input
esel = EventSelector(PrintFreq=100)
esel.Input = [# new
              "COLLECTION='Fill/MyCOL1' DATAFILE='PFN:ROOT_IO.etags' "
              "SVC='Gaudi::RootCnvSvc' SEL='(Ntrack>9 && Ntrack<20 && Energy<180)' "
              "FUN='Gaudi::Examples::EvtExtCollectionSelector'",
              # old
              "COLLECTION='Dir1/Dir2/Dir3/Collection' DATAFILE='PFN:ROOT_IO.tags' "
              "SVC='Gaudi::RootCnvSvc' SEL='(Ntrack>15)' "
              "FUN='Gaudi::Examples::EvtCollectionSelector'"]
evtColl = TagCollectionSvc("EvtTupleSvc")

# Application
app = ApplicationMgr(TopAlg=[ReadAlg()],
                     EvtMax=-1,
                     HistogramPersistency="NONE")
app.ExtSvc.append(evtColl)

# Verbosity
#SequencerTimerTool(OutputLevel=WARNING)

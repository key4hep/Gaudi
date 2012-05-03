"""
Create an Event Tag Collection based from the DST file prepared by the
"Prepare.py" options.
"""
from Gaudi.Configuration import *
from Configurables import GaudiPersistency, RndmGenSvc, TagCollectionSvc
from Configurables import Gaudi__Examples__EvtColAlg as EvtColAlg

GaudiPersistency()
FileCatalog(Catalogs = [ "xmlcatalog_file:EvtColsEx.xml" ])

# Input
esel = EventSelector(PrintFreq=1000)
esel.Input = ["DATAFILE='PFN:EvtColsEx.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'"]

# Output
evtColl = TagCollectionSvc("EvtTupleSvc")
evtColl.Output = ["EVTCOLS DATAFILE='PFN:EvtColsEx.tags' OPT='RECREATE' SVC='Gaudi::RootCnvSvc'"]

app = ApplicationMgr()
app.TopAlg = [EvtColAlg("Fill",
                        #Logical unit for Event Tag Collection
                        EvtColLUN="EVTCOLS")]
app.EvtMax = -1
app.ExtSvc += [RndmGenSvc(), evtColl]

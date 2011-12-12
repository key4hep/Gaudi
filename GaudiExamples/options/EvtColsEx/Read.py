"""
Read the Event Tag Collection
"""
from Gaudi.Configuration import *
from Configurables import GaudiPersistency, TagCollectionSvc

GaudiPersistency()
FileCatalog(Catalogs = [ "xmlcatalog_file:EvtColsEx.xml" ])

evtColl = TagCollectionSvc("EvtTupleSvc")

# Input
esel = EventSelector(PrintFreq=1)
esel.Input = ["COLLECTION='Fill/COL1' SEL='%s' "
              "DATAFILE='PFN:EvtColsEx.tags' "
              "SVC='Gaudi::RootCnvSvc' OPT='READ'" % sel
              for sel in ['runNum<10  && runNum>8',
                          'runNum<100 && flat>9.9',
                          'binom==7 || poisson==9']
              ]
# "COLLECTION='Fill/COL1' SEL='runNum<10  && runNum>8' DATAFILE='PFN:EvtColsEx.tags' SVC='Gaudi::RootEvtSelector' OPT='READ'"
# "COLLECTION='Fill/COL1' SEL='runNum<100 && flat>9.9' DATAFILE='PFN:EvtColsEx.tags' SVC='Gaudi::RootEvtSelector' OPT='READ'"
# "COLLECTION='Fill/COL1' SEL='binom==7 || poisson==9' DATAFILE='PFN:EvtColsEx.tags' SVC='Gaudi::RootEvtSelector' OPT='READ'"

app = ApplicationMgr()
app.EvtMax = -1
app.ExtSvc += [evtColl]

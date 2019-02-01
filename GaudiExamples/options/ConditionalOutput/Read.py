####################################################################
# Write a DST and a miniDST, including File Summary Records
####################################################################

from Gaudi.Configuration import *
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc, GaudiPersistency
from Configurables import GaudiTesting__GetDataObjectAlg as DataReader

# Output Levels
# MessageSvc(OutputLevel=VERBOSE)
# IncidentSvc(OutputLevel=DEBUG)
# RootCnvSvc(OutputLevel=INFO)

# Input setup
GaudiPersistency()

FileCatalog(Catalogs=["xmlcatalog_file:ConditionalOutput.xml"])

esel = EventSelector(OutputLevel=INFO, PrintFreq=1, FirstEvent=1)
esel.Input = [
    "DATAFILE='PFN:ConditionalOutput.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'"
]

# Application setup
app = ApplicationMgr()
# - Algorithms
app.TopAlg = [
    DataReader('DataReader', Paths=['A', 'B', 'C', 'D'], IgnoreMissing=True)
]
# - Events
app.EvtMax = -1
app.EvtSel = esel
app.HistogramPersistency = "NONE"

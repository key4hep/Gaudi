####################################################################
# Write a DST and a miniDST, including File Summary Records
####################################################################

from Gaudi.Configuration import *
from Configurables import GaudiPersistency
from Configurables import Gaudi__Examples__MultiInput__ReadAlg as ReadAlg

# I/O
GaudiPersistency()
esel = EventSelector()
esel.Input = [
    "DATAFILE='PFN:MI_Base.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'"
]
FileCatalog(Catalogs=["xmlcatalog_file:MultiInput.xml"])
# Algorithms
evtAlgs = GaudiSequencer(
    "EventAlgs",
    Members=[ReadAlg(AddressesFile='addresses.txt', OutputLevel=DEBUG)])

# Application setup
app = ApplicationMgr()
# - Algorithms
app.TopAlg = [evtAlgs]
# - Events
app.EvtMax = 100
app.HistogramPersistency = "NONE"

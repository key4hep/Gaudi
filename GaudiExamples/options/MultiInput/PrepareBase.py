# Write a dummy DST-like file.

from Gaudi.Configuration import *
from Configurables import GaudiPersistency
from Configurables import Gaudi__Examples__MultiInput__WriteAlg as WriteAlg

# Enable basic persistency-related settings
GaudiPersistency()

out = OutputStream()
out.ItemList = ["/Event#999"]
out.Output = "DATAFILE='PFN:MI_Base.dst' SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"

FileCatalog(Catalogs=["xmlcatalog_file:MultiInput.xml"])

app = ApplicationMgr()
# - I/O
app.OutStream += [out]
# - Algorithms
app.TopAlg = [WriteAlg()]
# - Events
app.EvtMax = 100
app.EvtSel = "NONE"  # do not use any event input
app.HistogramPersistency = "NONE"

"""
Prepare DST file to be used by EvtColsEx examples.
"""
from Gaudi.Configuration import *
from Configurables import GaudiPersistency

GaudiPersistency()
FileCatalog(Catalogs = [ "xmlcatalog_file:EvtColsEx.xml" ])

# Output setup
# - DST
dst           = OutputStream("RootDst")
dst.ItemList  = ["/Event#1"]
dst.Output    = "DATAFILE='PFN:EvtColsEx.dst'  SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"

# Application setup
app = ApplicationMgr()
# - I/O
app.OutStream += [ dst ]
# - Events
app.EvtMax   = 50000
app.EvtSel   = "NONE" # do not use any event input

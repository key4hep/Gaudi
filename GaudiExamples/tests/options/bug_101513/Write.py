####################################################################
# Produce a special DST file to trick IODataManager and expose
# bug #101513: Raw->Raw processing should not print an error
# https://savannah.cern.ch/bugs/?101513
####################################################################

from Gaudi.Configuration import *
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc, GaudiPersistency

# Output setup
dst           = OutputStream("RootDst")
dst.ItemList  = ["/Event#999"]
dst.Output    = "DATAFILE='PFN:bug_101513.dst'  SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"

# Special hacked file catalog
f = open("bug_101513.xml", "w")
f.write("""<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<!DOCTYPE POOLFILECATALOG SYSTEM "InMemory">
<POOLFILECATALOG>
  <File ID="B0101513-0000-0000-0000-000000000000">
    <physical>
      <pfn filetype="ROOT" name="bug_101513.dst"/>
    </physical>
    <logical/>
  </File>
</POOLFILECATALOG>
""")
f.close()

FileCatalog(Catalogs = [ "xmlcatalog_file:bug_101513.xml" ])

# Output Levels
#MessageSvc(OutputLevel=VERBOSE)
#IncidentSvc(OutputLevel=DEBUG)
#RootCnvSvc(OutputLevel=VERBOSE)

GaudiPersistency()

# Application setup
app = ApplicationMgr()
# - I/O
app.OutStream = [ dst ]
# - Algorithms
app.TopAlg = [ "WriteAlg" ]
# - Events
app.EvtMax   = 1
app.EvtSel   = "NONE" # do not use any event input
app.HistogramPersistency = "NONE"

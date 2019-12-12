#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
####################################################################
# Produce a special DST file to trick IODataManager and expose
# bug #101513: Raw->Raw processing should not print an error
# https://savannah.cern.ch/bugs/?101513
####################################################################

from Gaudi.Configuration import *
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc, GaudiPersistency

# Output setup
dst = OutputStream("RootDst")
dst.ItemList = ["/Event#999"]
dst.Output = "DATAFILE='PFN:bug_101513.dst'  SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"

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

FileCatalog(Catalogs=["xmlcatalog_file:bug_101513.xml"])

# Output Levels
# MessageSvc(OutputLevel=VERBOSE)
# IncidentSvc(OutputLevel=DEBUG)
# RootCnvSvc(OutputLevel=VERBOSE)

GaudiPersistency()

# Application setup
app = ApplicationMgr()
# - I/O
app.OutStream = [dst]
# - Algorithms
app.TopAlg = ["WriteAlg"]
# - Events
app.EvtMax = 1
app.EvtSel = "NONE"  # do not use any event input
app.HistogramPersistency = "NONE"

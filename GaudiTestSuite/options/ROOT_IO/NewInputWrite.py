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
# Write a DST and a miniDST, including File Summary Records
####################################################################

from Configurables import Gaudi__Hive__FetchLeavesFromFile as FetchLeavesFromFile
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc
from Configurables import GaudiPersistency
from Gaudi.Configuration import *

# Output setup
# - DST
dst = CopyInputStream("NewRootDst")
dst.Output = "DATAFILE='PFN:NEWROOTIO.dst'  SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"

FileCatalog(Catalogs=["xmlcatalog_file:NEWROOTIO.xml"])

# Output Levels
MessageSvc(OutputLevel=VERBOSE)
IncidentSvc(OutputLevel=DEBUG)
RootCnvSvc(OutputLevel=INFO)
AlgExecStateSvc(OutputLevel=INFO)

GaudiPersistency()

esel = EventSelector(OutputLevel=DEBUG, PrintFreq=10, FirstEvent=1)
esel.Input = [
    "DATAFILE='PFN:ROOTIO.2.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'",
]

# Application setup
app = ApplicationMgr()
# - I/O
app.OutStream += [dst]
# - Algorithms
fetch = FetchLeavesFromFile("NewFetch")
app.TopAlg = [fetch]
# - Events
app.EvtMax = -1
app.HistogramPersistency = "NONE"

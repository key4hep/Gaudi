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

from Configurables import Gaudi__RootCnvSvc as RootCnvSvc
from Configurables import GaudiPersistency
from Gaudi.Configuration import *

# Output setup
# - DST
dst = OutputStream("RootDst")
dst.ItemList = ["/Event#999"]
dst.Output = "DATAFILE='PFN:ROOTIO.2.dst'  SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"

FileCatalog(Catalogs=["xmlcatalog_file:ROOTIO.2.xml"])

# Output Levels
MessageSvc(OutputLevel=VERBOSE)
IncidentSvc(OutputLevel=DEBUG)
RootCnvSvc(OutputLevel=INFO)
AlgExecStateSvc(OutputLevel=INFO)

GaudiPersistency()

# Application setup
app = ApplicationMgr()
# - I/O
app.OutStream += [dst]
# - Algorithms
app.TopAlg = ["WriteAlg"]
# - Events
app.EvtMax = 10
app.EvtSel = "NONE"  # do not use any event input
app.HistogramPersistency = "NONE"

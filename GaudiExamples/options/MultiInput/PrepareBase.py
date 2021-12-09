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
# Write a dummy DST-like file.

from Configurables import Gaudi__Examples__MultiInput__WriteAlg as WriteAlg
from Configurables import GaudiPersistency
from Gaudi.Configuration import *

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

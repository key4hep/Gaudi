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
"""
Prepare DST file to be used by EvtColsEx examples.
"""

from Configurables import GaudiPersistency
from Gaudi.Configuration import *

GaudiPersistency()
FileCatalog(Catalogs=["xmlcatalog_file:EvtColsEx.xml"])

# Output setup
# - DST
dst = OutputStream("RootDst")
dst.ItemList = ["/Event#1"]
dst.Output = "DATAFILE='PFN:EvtColsEx.dst'  SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"

# Application setup
app = ApplicationMgr()
# - I/O
app.OutStream += [dst]
# - Events
app.EvtMax = 50000
app.EvtSel = "NONE"  # do not use any event input

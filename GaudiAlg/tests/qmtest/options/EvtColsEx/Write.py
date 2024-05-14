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
Create an Event Tag Collection based from the DST file prepared by the
"Prepare.py" options.
"""

from Configurables import Gaudi__TestSuite__EvtColAlg as EvtColAlg
from Configurables import GaudiPersistency, RndmGenSvc, TagCollectionSvc
from Gaudi.Configuration import *

GaudiPersistency()
FileCatalog(Catalogs=["xmlcatalog_file:EvtColsEx.xml"])

# Input
esel = EventSelector(PrintFreq=1000)
esel.Input = ["DATAFILE='PFN:EvtColsEx.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'"]

# Output
evtColl = TagCollectionSvc("EvtTupleSvc")
evtColl.Output = [
    "EVTCOLS DATAFILE='PFN:EvtColsEx.tags' OPT='RECREATE' SVC='Gaudi::RootCnvSvc'"
]

app = ApplicationMgr()
app.TopAlg = [
    EvtColAlg(
        "Fill",
        # Logical unit for Event Tag Collection
        EvtColLUN="EVTCOLS",
    )
]
app.EvtMax = -1
app.ExtSvc += [RndmGenSvc(), evtColl]

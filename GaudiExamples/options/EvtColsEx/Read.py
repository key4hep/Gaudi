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
Read the Event Tag Collection
"""
from Configurables import GaudiPersistency, TagCollectionSvc
from Gaudi.Configuration import *

GaudiPersistency()
FileCatalog(Catalogs=["xmlcatalog_file:EvtColsEx.xml"])

evtColl = TagCollectionSvc("EvtTupleSvc")

# Input
esel = EventSelector(PrintFreq=1)
esel.Input = [
    "COLLECTION='Fill/COL1' SEL='%s' "
    "DATAFILE='PFN:EvtColsEx.tags' "
    "SVC='Gaudi::RootCnvSvc' OPT='READ'" % sel
    for sel in [
        "runNum<10  && runNum>8",
        "runNum<100 && flat>9.9",
        "binom==7 || poisson==9",
    ]
]
# "COLLECTION='Fill/COL1' SEL='runNum<10  && runNum>8' DATAFILE='PFN:EvtColsEx.tags' SVC='Gaudi::RootEvtSelector' OPT='READ'"
# "COLLECTION='Fill/COL1' SEL='runNum<100 && flat>9.9' DATAFILE='PFN:EvtColsEx.tags' SVC='Gaudi::RootEvtSelector' OPT='READ'"
# "COLLECTION='Fill/COL1' SEL='binom==7 || poisson==9' DATAFILE='PFN:EvtColsEx.tags' SVC='Gaudi::RootEvtSelector' OPT='READ'"

app = ApplicationMgr()
app.EvtMax = -1
app.ExtSvc += [evtColl]

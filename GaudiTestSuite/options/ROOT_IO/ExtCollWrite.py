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
from Configurables import Gaudi__TestSuite__ExtendedEvtCol as ExtendedEvtCol
from Configurables import GaudiPersistency
from Gaudi.Configuration import *

# persistency setup
GaudiPersistency()

FileCatalog(Catalogs=["xmlcatalog_file:ROOTIO.xml"])

# Input
esel = EventSelector(PrintFreq=100)
esel.Input = [
    "DATAFILE='PFN:ROOTIO.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'",
    "DATAFILE='PFN:ROOTIO.mdst' SVC='Gaudi::RootEvtSelector' OPT='READ'",
]

# Output
evtColl = TagCollectionSvc("EvtTupleSvc")
evtColl.Output = [
    "EXTEVT DATAFILE='PFN:ROOT_IO.etags' OPT='RECREATE' SVC='Gaudi::RootCnvSvc'"
]

# Algorithms
algs = Gaudi__Sequencer(
    "EventAlgs",
    Members=[
        ExtendedEvtCol("Fill", EvtColLUN="EXTEVT")
    ],  # Logical unit for Event Tag Collection
    VetoObjects=["FSR"],
)

# Application
app = ApplicationMgr(TopAlg=[algs], EvtMax=-1, HistogramPersistency="NONE")
app.ExtSvc.append(evtColl)

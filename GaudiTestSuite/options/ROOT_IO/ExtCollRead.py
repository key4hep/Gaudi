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
from Configurables import GaudiPersistency, ReadAlg, SequencerTimerTool
from Gaudi.Configuration import *

# Basic configuration for Gaudi persistency
GaudiPersistency()

FileCatalog(Catalogs=["xmlcatalog_file:ROOTIO.xml"])

# Input
esel = EventSelector(PrintFreq=100)
esel.Input = [  # new
    "COLLECTION='Fill/MyCOL1' DATAFILE='PFN:ROOT_IO.etags' "
    "SVC='Gaudi::RootCnvSvc' SEL='(Ntrack>9 && Ntrack<20 && Energy<180)' "
    "FUN='Gaudi::TestSuite::EvtExtCollectionSelector'",
    # old
    "COLLECTION='Dir1/Dir2/Dir3/Collection' DATAFILE='PFN:ROOT_IO.tags' "
    "SVC='Gaudi::RootCnvSvc' SEL='(Ntrack>15)' "
    "FUN='Gaudi::TestSuite::EvtCollectionSelector'",
]
evtColl = TagCollectionSvc("EvtTupleSvc")

# Application
app = ApplicationMgr(TopAlg=[ReadAlg()], EvtMax=-1, HistogramPersistency="NONE")
app.ExtSvc.append(evtColl)

# Verbosity
# SequencerTimerTool(OutputLevel=WARNING)

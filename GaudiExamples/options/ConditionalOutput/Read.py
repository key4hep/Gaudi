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

from Gaudi.Configuration import *
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc, GaudiPersistency
from Configurables import GaudiTesting__GetDataObjectAlg as DataReader

# Output Levels
# MessageSvc(OutputLevel=VERBOSE)
# IncidentSvc(OutputLevel=DEBUG)
# RootCnvSvc(OutputLevel=INFO)

# Input setup
GaudiPersistency()

FileCatalog(Catalogs=["xmlcatalog_file:ConditionalOutput.xml"])

esel = EventSelector(OutputLevel=INFO, PrintFreq=1, FirstEvent=1)
esel.Input = [
    "DATAFILE='PFN:ConditionalOutput.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'"
]

# Application setup
app = ApplicationMgr()
# - Algorithms
app.TopAlg = [
    DataReader('DataReader', Paths=['A', 'B', 'C', 'D'], IgnoreMissing=True)
]
# - Events
app.EvtMax = -1
app.EvtSel = esel
app.HistogramPersistency = "NONE"

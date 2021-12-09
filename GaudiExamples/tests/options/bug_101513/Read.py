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
# Trick the IODataManager to expose bug #101513:
#   Raw->Raw processing should not print an error
# https://savannah.cern.ch/bugs/?101513
####################################################################

import shutil

from Configurables import FileRecordDataSvc
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc
from Configurables import GaudiPersistency, ReadAlg, ReadTES
from Gaudi.Configuration import *

shutil.copy("bug_101513.dst", "B0101513-0000-0000-0000-000000000000")

# I/O
GaudiPersistency()
FileCatalog(Catalogs=[])
esel = EventSelector(PrintFreq=50, FirstEvent=1)
esel.Input = [
    "DATAFILE='PFN:B0101513-0000-0000-0000-000000000000'  SVC='Gaudi::RootEvtSelector' OPT='READ'"
]

# Application setup
app = ApplicationMgr()
# - Algorithms
app.TopAlg = []
# - Events
app.EvtMax = 1
app.HistogramPersistency = "NONE"

# RootCnvSvc(OutputLevel=VERBOSE)
# ChronoStatSvc(OutputLevel=WARNING)
# SequencerTimerTool(OutputLevel=WARNING)
# MessageSvc(OutputLevel=VERBOSE)

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

from Configurables import Gaudi__Examples__MultiInput__ReadAlg as ReadAlg
from Configurables import GaudiPersistency
from Gaudi.Configuration import *

# I/O
GaudiPersistency()
esel = EventSelector()
esel.Input = ["DATAFILE='PFN:MI_Base.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'"]
FileCatalog(Catalogs=["xmlcatalog_file:MultiInput.xml"])
# Algorithms
evtAlgs = Gaudi__Sequencer(
    "EventAlgs", Members=[ReadAlg(AddressesFile="addresses.txt", OutputLevel=DEBUG)]
)

# Application setup
app = ApplicationMgr()
# - Algorithms
app.TopAlg = [evtAlgs]
# - Events
app.EvtMax = 100
app.HistogramPersistency = "NONE"

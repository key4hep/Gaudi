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
from Configurables import GaudiTesting__OddEventsFilter as OddFilter
from Configurables import GaudiTesting__EvenEventsFilter as EvenFilter
from Configurables import GaudiTesting__PutDataObjectAlg as DataCreator

# create data in the TES
creator = DataCreator('DataCreator')
creator.Paths = ['A', 'B', 'C', 'D']

# Filter algorithms
oddEvts = OddFilter('OddEvents')
evenEvts = EvenFilter('EvenEvents')

# Output setup
# - DST
stream = OutputStream()
stream.Output = "DATAFILE='PFN:ConditionalOutput.dst'  SVC='Gaudi::RootCnvSvc' OPT='RECREATE'"
#stream.ItemList = ['A', 'B', 'C']
stream.AlgDependentItemList[oddEvts.name()] = ['/Event/A#1', '/Event/B#1']
stream.AlgDependentItemList[evenEvts.name()] = ['/Event/A#1', '/Event/C#1']

FileCatalog(Catalogs=["xmlcatalog_file:ConditionalOutput.xml"])

# Output Levels
MessageSvc(OutputLevel=VERBOSE)
IncidentSvc(OutputLevel=DEBUG)
RootCnvSvc(OutputLevel=INFO)

GaudiPersistency()

# Application setup
app = ApplicationMgr()
# - I/O
app.OutStream += [stream]
# - Algorithms
app.TopAlg = [creator, oddEvts, evenEvts]
# - Events
app.EvtMax = 10
app.EvtSel = "NONE"  # do not use any event input
app.HistogramPersistency = "NONE"

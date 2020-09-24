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
from Gaudi.Configuration import *

AuditorSvc().Auditors = ['ChronoAuditor']
MessageSvc().OutputLevel = INFO
RootHistSvc('RootHistSvc').OutputFile = 'histo.root'

#HistogramSvc('HistogramDataSvc').Input = [ "InFile DATAFILE='../data/input.hbook' TYP='HBOOK'" ]

from Configurables import (HistoTimingAlgDA as CounterHistoTimingDA,
                           HistoTimingAlgIA as CounterHistoTimingIA,
                           HistoTimingAlgD as CounterHistoTimingD,
                           HistoTimingAlgI as CounterHistoTimingI)

seq = GaudiSequencer("TimingSeq", MeasureTime=True)
seq.Members = [
    HistoTimingAlg("DirectFill", UseLookup=False),
    HistoTimingAlg("LookupFill", UseLookup=True),
    CounterHistoTimingDA("CounterTimingDA"),
    CounterHistoTimingIA("CounterTimingIA"),
    CounterHistoTimingD("CounterTimingA"),
    CounterHistoTimingI("CounterTimingI"),
]
for alg in seq.Members:
    alg.NumTracks = 20
    alg.NumHistos = 30
    alg.OutputLevel = DEBUG

ApplicationMgr(
    EvtMax=50000, EvtSel='NONE', HistogramPersistency='ROOT', TopAlg=[seq])

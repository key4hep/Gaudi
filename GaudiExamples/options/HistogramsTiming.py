from Gaudi.Configuration import *

AuditorSvc().Auditors = ['ChronoAuditor']
MessageSvc().OutputLevel = INFO
RootHistSvc('RootHistSvc').OutputFile = 'histo.root'

#HistogramSvc('HistogramDataSvc').Input = [ "InFile DATAFILE='../data/input.hbook' TYP='HBOOK'" ]

from Configurables import (
    HistoTimingAlg, Gaudi__Examples__R7__HistoTimingAlg as R7HistoTiming,
    Gaudi__Examples__Boost__HistoTimingAlg as BoostHistoTiming)

seq = GaudiSequencer("TimingSeq", MeasureTime=True)
seq.Members = [
    HistoTimingAlg("DirectFill", UseLookup=False),
    HistoTimingAlg("LookupFill", UseLookup=True),
    R7HistoTiming("R7Timing"),
    BoostHistoTiming("BoostTiming"),
]
for alg in seq.Members:
    alg.NumTracks = 20
    alg.NumHistos = 30
    alg.OutputLevel = DEBUG

ApplicationMgr(
    EvtMax=50000, EvtSel='NONE', HistogramPersistency='ROOT', TopAlg=[seq])

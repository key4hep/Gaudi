from Gaudi.Configuration import *

AuditorSvc().Auditors = ['ChronoAuditor']
MessageSvc().OutputLevel = INFO
RootHistSvc('RootHistSvc').OutputFile = 'histo.root'

#HistogramSvc('HistogramDataSvc').Input = [ "InFile DATAFILE='../data/input.hbook' TYP='HBOOK'" ]

from Configurables import HistoTimingAlg

seq = GaudiSequencer("TimingSeq", MeasureTime = True)
seq.Members = [HistoTimingAlg("DirectFill"), HistoTimingAlg("LookupFill")]
HistoTimingAlg("DirectFill").UseLookup = False
HistoTimingAlg("LookupFill").UseLookup = True
for alg in seq.Members:
    alg.NumTracks = 20
    alg.NumHistos = 30
    alg.OutputLevel = DEBUG

ApplicationMgr( EvtMax = 50000,
                EvtSel = 'NONE',
                HistogramPersistency = 'ROOT',
                TopAlg = [seq] )

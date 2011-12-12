from Gaudi.Configuration import *

AuditorSvc().Auditors = ['ChronoAuditor']
MessageSvc().OutputLevel = INFO
RootHistSvc('RootHistSvc').OutputFile = 'histo.root'

HistogramSvc('HistogramDataSvc').Input = [ "InFile DATAFILE='../data/input.hbook' TYP='HBOOK'" ]

#from GaudiExamples.GaudiExamplesConf import GaudiHistoAlgorithm
from Configurables import GaudiHistoAlgorithm

alg = GaudiHistoAlgorithm('SimpleHistos',
                           HistoPrint = True,
                           OutputLevel = DEBUG)

ApplicationMgr( EvtMax = 5000,
                EvtSel = 'NONE',
                HistogramPersistency = 'ROOT',
                TopAlg = [alg] )

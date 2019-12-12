from Gaudi.Configuration import *

AuditorSvc().Auditors = ['ChronoAuditor']
MessageSvc().OutputLevel = INFO
RootHistSvc('RootHistSvc').OutputFile = 'histo.root'

HistogramSvc('HistogramDataSvc').Input = [
    "InFile DATAFILE='../data/input.hbook' TYP='HBOOK'"
]

#from GaudiExamples.GaudiExamplesConf import GaudiHistoAlgorithm
from Configurables import GaudiHistoAlgorithm
from Configurables import Gaudi__Examples__R7__GaudiHistoAlgorithm as R7HistoAlg
from Configurables import Gaudi__Examples__Boost__GaudiHistoAlgorithm as BoostHistoAlg

algs = [
    GaudiHistoAlgorithm('SimpleHistos', HistoPrint=True, OutputLevel=DEBUG),
    R7HistoAlg('SimpleR7Histos', OutputLevel=DEBUG),
    BoostHistoAlg('SimpleBoostHistos', OutputLevel=DEBUG),
]

ApplicationMgr(
    EvtMax=50000, EvtSel='NONE', HistogramPersistency='ROOT', TopAlg=algs)

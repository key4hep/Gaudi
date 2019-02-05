###############################################################
# Job options file
# ==============================================================
from Gaudi.Configuration import *
from Configurables import MyGaudiAlgorithm

from Configurables import GaudiExamplesCommonConf
GaudiExamplesCommonConf()

mygalg = MyGaudiAlgorithm('MyGaudiAlg', OutputLevel=DEBUG)
mygalg.PrivToolHandle.String = "Is a private tool"

print mygalg

mygalg.tracks.Path = 'BestTracks/Tracks'
mygalg.hits.Path = 'Rec/Hits'
mygalg.raw.AlternativePaths = ['/copy/RAW', '/Rec/RAW', '/DAQ/RAW']

mygalg.RootInTES = '/Skim'

print mygalg

ApplicationMgr(
    EvtMax=10,
    EvtSel='NONE',
    HistogramPersistency='NONE',
    TopAlg=[mygalg],
    OutputLevel=DEBUG)

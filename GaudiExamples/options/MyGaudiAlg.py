###############################################################
# Job options file
#==============================================================
from Gaudi.Configuration import *
from Configurables import MyGaudiAlgorithm

from Configurables import GaudiExamplesCommonConf
GaudiExamplesCommonConf()

mygalg = MyGaudiAlgorithm('MyGaudiAlg',
                          OutputLevel = DEBUG )
mygalg.PrivToolHandle.String = "Is a private tool"

print mygalg

mygalg.Inputs.tracks.Path = 'BestTracks/Tracks'
mygalg.Inputs.hits.Path = 'Rec/Hits'
mygalg.Inputs.raw.AlternativePaths.insert(0, '/copy/RAW')

mygalg.RootInTES = '/Skim'

print mygalg

ApplicationMgr( EvtMax = 10,
                EvtSel = 'NONE',
                HistogramPersistency = 'NONE',
                TopAlg = [mygalg],
                OutputLevel = DEBUG )


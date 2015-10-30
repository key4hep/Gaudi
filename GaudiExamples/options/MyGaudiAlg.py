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

mygalg.DataInputs.tracks.Path = 'BestTracks/Tracks'
mygalg.DataInputs.hits.Path = 'Rec/Hits'
mygalg.DataInputs.raw.AlternativePaths.insert(0, '/copy/RAW')

mygalg.RootInTES = '/Skim'

print mygalg

ApplicationMgr( EvtMax = 10,
                EvtSel = 'NONE',
                HistogramPersistency = 'NONE',
                TopAlg = [mygalg],
                OutputLevel = DEBUG )


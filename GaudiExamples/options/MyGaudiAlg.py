###############################################################
# Job options file
#==============================================================
from Gaudi.Configuration import *
from Configurables import MyGaudiAlgorithm

from Configurables import GaudiExamplesCommonConf
GaudiExamplesCommonConf()

mygalg = MyGaudiAlgorithm('MyGaudiAlg')
mygalg.PrivToolHandle.String = "Is a private tool"

print mygalg

mygalg.InputDataObjects.tracks.address = 'BestTracks/Tracks'
mygalg.InputDataObjects.hits.address = 'Rec/Hits'
mygalg.InputDataObjects.raw.alternativeAddresses.insert(0, '/copy/RAW')

mygalg.RootInTES = '/Skim'

print mygalg

ApplicationMgr( EvtMax = 10,
                EvtSel = 'NONE',
                HistogramPersistency = 'NONE',
                TopAlg = [mygalg],
                OutputLevel = DEBUG )


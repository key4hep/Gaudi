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

mygalg.InputDataItems.tracks.address = 'BestTracks/Tracks'
mygalg.InputDataItems.hits.address = 'Rec/Hits'
mygalg.InputDataItems.raw.alternativeAddresses.insert(0, '/copy/RAW')

mygalg.RootInTES = '/Skim'

print mygalg

ApplicationMgr( EvtMax = 10,
                EvtSel = 'NONE',
                HistogramPersistency = 'NONE',
                TopAlg = [mygalg],
                OutputLevel = DEBUG )


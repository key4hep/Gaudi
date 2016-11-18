####################################################################
# Write a DST and a miniDST, including File Summary Records
####################################################################

from Gaudi.Configuration import *
from Configurables import Gaudi__Examples__IntDataProducer as IntDataProducer
from Configurables import Gaudi__Examples__FloatDataConsumer as FloatDataConsumer
from Configurables import Gaudi__Examples__IntDataConsumer as IntDataConsumer
from Configurables import Gaudi__Examples__IntToFloatData as IntToFloatData

# Application setup
app = ApplicationMgr()
# - Algorithms
app.TopAlg = [ IntDataProducer(), IntDataConsumer(), IntToFloatData(), FloatDataConsumer() ]
# - Events
app.EvtMax   = 2
app.EvtSel   = "NONE"
app.HistogramPersistency = "NONE"


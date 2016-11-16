####################################################################
# Write a DST and a miniDST, including File Summary Records
####################################################################

from Gaudi.Configuration import *
from Configurables import Gaudi__Examples__IntProduceData as IntProduceData
from Configurables import Gaudi__Examples__FloatConsumeData as FloatConsumeData
from Configurables import Gaudi__Examples__IntConsumeData as IntConsumeData
from Configurables import Gaudi__Examples__IntToFloatData as IntToFloatData

# Application setup
app = ApplicationMgr()
# - Algorithms
app.TopAlg = [ IntProduceData, IntConsumeData, IntToFloat, FloatConsumeData ]
# - Events
app.EvtMax   = 10
app.HistogramPersistency = "NONE"


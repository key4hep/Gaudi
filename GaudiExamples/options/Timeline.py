#
# Write a DST and a miniDST, including File Summary Records
#

from Gaudi.Configuration import *
from Configurables import Gaudi__Examples__IntDataProducer as IntDataProducer
from Configurables import Gaudi__Examples__FloatDataConsumer as FloatDataConsumer
from Configurables import Gaudi__Examples__IntDataConsumer as IntDataConsumer
from Configurables import Gaudi__Examples__IntToFloatData as IntToFloatData
from Configurables import Gaudi__Examples__IntIntToFloatFloatData as IntIntToFloatFloatData
from Configurables import TimelineSvc

# Application setup
app = ApplicationMgr()
# - Algorithms
OtherIntDataProducer = IntDataProducer('OtherIntDataProducer')
OtherIntDataProducer.OutputLocation = "/Event/MyOtherInt"
app.TopAlg = [IntDataProducer("IntDataProducer"),
              OtherIntDataProducer,
              IntDataConsumer("IntDataConsumer"),
              IntToFloatData("IntToFloatData"),
              IntIntToFloatFloatData("IntIntToFloatFloatData"),
              FloatDataConsumer("FloatDataConsumer")]
# - Events
app.EvtMax = 2
app.EvtSel = "NONE"
app.HistogramPersistency = "NONE"
# - Activate timeline service
TimelineSvc(RecordTimeline=True,
            DumpTimeline=True, TimelineFile='myTimeline.csv')

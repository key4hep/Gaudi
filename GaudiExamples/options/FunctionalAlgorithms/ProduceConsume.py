####################################################################
# Write a DST and a miniDST, including File Summary Records
####################################################################

from Gaudi.Configuration import *
from Configurables import Gaudi__Examples__IntDataProducer as IntDataProducer
from Configurables import Gaudi__Examples__FloatDataConsumer as FloatDataConsumer
from Configurables import Gaudi__Examples__IntDataConsumer as IntDataConsumer
from Configurables import Gaudi__Examples__IntToFloatData as IntToFloatData
from Configurables import Gaudi__Examples__IntIntToFloatFloatData as IntIntToFloatFloatData
from Configurables import Gaudi__Examples__ContextConsumer as ContextConsumer
from Configurables import Gaudi__Examples__ContextIntConsumer as ContextIntConsumer
from Configurables import Gaudi__Examples__VectorDoubleProducer as VectorDoubleProducer
from Configurables import Gaudi__Examples__FrExpTransformer as FrExpTransformer
from Configurables import Gaudi__Examples__LdExpTransformer as LdExpTransformer
from Configurables import Gaudi__Examples__OptFrExpTransformer as OptFrExpTransformer
from Configurables import Gaudi__Examples__OptLdExpTransformer as OptLdExpTransformer

# Application setup
app = ApplicationMgr()
# - Algorithms
OtherIntDataProducer = IntDataProducer('OtherIntDataProducer')
OtherIntDataProducer.OutputLocation = "/Event/MyOtherInt"
app.TopAlg = [IntDataProducer("IntDataProducer"), OtherIntDataProducer,
              IntDataConsumer("IntDataConsumer"), IntToFloatData(
                  "IntToFloatData"),
              IntIntToFloatFloatData("IntIntToFloatFloatData"), FloatDataConsumer(
                  "FloatDataConsumer"),
              ContextConsumer("ContextConsumer"),
              ContextIntConsumer("ContextIntConsumer"),
              VectorDoubleProducer("VectorDoubleProducer"),
              FrExpTransformer("FrExpTransformer"), LdExpTransformer(
                  "LdExpTransfomer"),
              OptFrExpTransformer("OptFrExpTransformer"), OptLdExpTransformer(
                  "OptLdExpTransformer")
              ]
# - Events
app.EvtMax = 2
app.EvtSel = "NONE"
app.HistogramPersistency = "NONE"

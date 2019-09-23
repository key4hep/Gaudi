####################################################################
# Write a DST and a miniDST, including File Summary Records
####################################################################
from GaudiKernel.DataObjectHandleBase import DataObjectHandleBase
from Gaudi.Configuration import *
from Configurables import Gaudi__Examples__IntDataProducer as IntDataProducer
from Configurables import Gaudi__Examples__VectorDataProducer as VectorDataProducer
from Configurables import Gaudi__Examples__FloatDataConsumer as FloatDataConsumer
from Configurables import Gaudi__Examples__IntDataConsumer as IntDataConsumer
from Configurables import Gaudi__Examples__IntToFloatData as IntToFloatData
from Configurables import Gaudi__Examples__IntIntToFloatFloatData as IntIntToFloatFloatData
from Configurables import Gaudi__Examples__IntVectorsToIntVector as IntVectorsToIntVector
from Configurables import Gaudi__Examples__ContextConsumer as ContextConsumer
from Configurables import Gaudi__Examples__ContextIntConsumer as ContextIntConsumer
from Configurables import Gaudi__Examples__VectorDoubleProducer as VectorDoubleProducer
from Configurables import Gaudi__Examples__FrExpTransformer as FrExpTransformer
from Configurables import Gaudi__Examples__LdExpTransformer as LdExpTransformer
from Configurables import Gaudi__Examples__OptFrExpTransformer as OptFrExpTransformer
from Configurables import Gaudi__Examples__OptLdExpTransformer as OptLdExpTransformer
from Configurables import Gaudi__Examples__CountingConsumer as CountingConsumer
from Configurables import EvtStoreSvc
# Application setup
app = ApplicationMgr()
#app.ExtSvc = [ EvtStoreSvc("EventDataSvc",OutputLevel=DEBUG ) ]
app.ExtSvc = [EvtStoreSvc("EventDataSvc")]

types = []

for configurable in [
        IntDataProducer, VectorDataProducer, FloatDataConsumer,
        IntDataConsumer, IntToFloatData, IntIntToFloatFloatData,
        IntVectorsToIntVector, ContextConsumer, ContextIntConsumer,
        VectorDoubleProducer, FrExpTransformer, LdExpTransformer,
        OptFrExpTransformer, OptLdExpTransformer, CountingConsumer
]:
    props = configurable.getDefaultProperties()
    for prop in (props[propname] for propname in sorted(props)):
        if isinstance(prop, DataObjectHandleBase):
            types.append(prop.type())

# check that the type information is passed to python correctly
print(types)

# - Algorithms
OtherIntDataProducer = IntDataProducer('OtherIntDataProducer')
OtherIntDataProducer.OutputLocation = "/Event/MyOtherInt"
VectorDataProducer1 = VectorDataProducer(
    "VectorDataProducer1", OutputLocation="/Event/IntVector1")
VectorDataProducer2 = VectorDataProducer(
    "VectorDataProducer2", OutputLocation="/Event/IntVector2")
app.TopAlg = [
    IntDataProducer("IntDataProducer"), OtherIntDataProducer,
    IntDataConsumer("IntDataConsumer"),
    IntToFloatData("IntToFloatData"),
    IntIntToFloatFloatData("IntIntToFloatFloatData"),
    FloatDataConsumer("FloatDataConsumer"),
    ContextConsumer("ContextConsumer"),
    ContextIntConsumer("ContextIntConsumer"),
    VectorDoubleProducer("VectorDoubleProducer"), VectorDataProducer1,
    VectorDataProducer2,
    IntVectorsToIntVector(
        "IntVectorsToIntVector",
        InputLocations=[
            str(VectorDataProducer1.OutputLocation),
            str(VectorDataProducer2.OutputLocation)
        ]),
    FrExpTransformer("FrExpTransformer"),
    LdExpTransformer("LdExpTransfomer"),
    OptFrExpTransformer("OptFrExpTransformer"),
    OptLdExpTransformer("OptLdExpTransformer"),
    CountingConsumer("CountingConsumer")
]
# - Events
app.EvtMax = 2
app.EvtSel = "NONE"
app.HistogramPersistency = "NONE"

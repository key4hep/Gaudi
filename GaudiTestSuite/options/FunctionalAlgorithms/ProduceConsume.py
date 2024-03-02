#####################################################################################
# (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
####################################################################
# Write a DST and a miniDST, including File Summary Records
####################################################################
from Configurables import EvtStoreSvc
from Configurables import Gaudi__Monitoring__MessageSvcSink as MessageSvcSink
from Configurables import Gaudi__TestSuite__ContextConsumer as ContextConsumer
from Configurables import Gaudi__TestSuite__ContextIntConsumer as ContextIntConsumer
from Configurables import Gaudi__TestSuite__CountingConsumer as CountingConsumer
from Configurables import Gaudi__TestSuite__FloatDataConsumer as FloatDataConsumer
from Configurables import Gaudi__TestSuite__FrExpTransformer as FrExpTransformer
from Configurables import Gaudi__TestSuite__IntDataConsumer as IntDataConsumer
from Configurables import Gaudi__TestSuite__IntDataProducer as IntDataProducer
from Configurables import (
    Gaudi__TestSuite__IntIntToFloatFloatData as IntIntToFloatFloatData,
)
from Configurables import Gaudi__TestSuite__IntToFloatData as IntToFloatData
from Configurables import Gaudi__TestSuite__IntVectorsMerger as IntVectorsMerger
from Configurables import (
    Gaudi__TestSuite__IntVectorsMergingConsumer as IntVectorsMergingConsumer,
)
from Configurables import Gaudi__TestSuite__IntVectorsToInts as IntVectorsToInts
from Configurables import (
    Gaudi__TestSuite__IntVectorsToIntVector as IntVectorsToIntVector,
)
from Configurables import Gaudi__TestSuite__LdExpTransformer as LdExpTransformer
from Configurables import Gaudi__TestSuite__MyConsumerTool as MyConsumerTool
from Configurables import Gaudi__TestSuite__MyExampleTool as MyExampleTool
from Configurables import Gaudi__TestSuite__OpaqueProducer as OpaqueProducer
from Configurables import Gaudi__TestSuite__OptFrExpTransformer as OptFrExpTransformer
from Configurables import Gaudi__TestSuite__OptLdExpTransformer as OptLdExpTransformer
from Configurables import Gaudi__TestSuite__SDataProducer as SDataProducer
from Configurables import Gaudi__TestSuite__ShrdPtrConsumer as ShrdPtrConsumer
from Configurables import Gaudi__TestSuite__ShrdPtrProducer as ShrdPtrProducer
from Configurables import Gaudi__TestSuite__SRangesToIntVector as SRangesToIntVector
from Configurables import Gaudi__TestSuite__ToolConsumer as ToolConsumer
from Configurables import Gaudi__TestSuite__TwoDMerger as TwoDMerger
from Configurables import Gaudi__TestSuite__VectorDataProducer as VectorDataProducer
from Configurables import Gaudi__TestSuite__VectorDoubleProducer as VectorDoubleProducer
from Gaudi.Configuration import *
from GaudiKernel.DataHandle import DataHandle

# Application setup
app = ApplicationMgr()
app.ExtSvc = [EvtStoreSvc("EventDataSvc"), MessageSvcSink()]

types = []

# this printout is useful to check that the type information is passed to python correctly
print("---\n# List of input and output types by class")
for configurable in sorted(
    [
        ToolConsumer,
        IntDataProducer,
        VectorDataProducer,
        FloatDataConsumer,
        IntDataConsumer,
        IntToFloatData,
        IntIntToFloatFloatData,
        IntVectorsToIntVector,
        ContextConsumer,
        ContextIntConsumer,
        VectorDoubleProducer,
        FrExpTransformer,
        LdExpTransformer,
        OptFrExpTransformer,
        OptLdExpTransformer,
        CountingConsumer,
        ShrdPtrProducer,
        ShrdPtrConsumer,
    ],
    key=lambda c: c.getType(),
):
    print('"{}":'.format(configurable.getType()))
    props = configurable.getDefaultProperties()
    for propname, prop in sorted(props.items()):
        if isinstance(prop, DataHandle):
            types.append(prop.type())
            print('  {}: "{}"'.format(propname, prop.type()))
print("---")

# - Algorithms

OtherIntDataProducer = IntDataProducer("OtherIntDataProducer")
OtherIntDataProducer.OutputLocation = "/Event/MyOtherInt"
VectorDataProducer1 = VectorDataProducer(
    "VectorDataProducer1", OutputLocation="/Event/IntVector1"
)
VectorDataProducer2 = VectorDataProducer(
    "VectorDataProducer2", OutputLocation="/Event/IntVector2"
)
SDataProducer1 = SDataProducer("SDataProducer1", OutputLocation="/Event/S1", j=3)
SDataProducer2 = SDataProducer("SDataProducer2", OutputLocation="/Event/S2", j=10)
app.TopAlg = [
    IntDataProducer("IntDataProducer"),
    OtherIntDataProducer,
    IntDataConsumer("IntDataConsumer"),
    ToolConsumer("MyToolConsumer", MyTool=MyExampleTool(Message="Hello World!!!")),
    ToolConsumer(
        "MyBoundToolConsumer", MyTool=MyConsumerTool(MyInt="/Event/MyOtherInt")
    ),
    IntToFloatData("IntToFloatData"),
    IntIntToFloatFloatData("IntIntToFloatFloatData"),
    FloatDataConsumer("FloatDataConsumer"),
    ContextConsumer("ContextConsumer"),
    ContextIntConsumer("ContextIntConsumer"),
    VectorDoubleProducer("VectorDoubleProducer"),
    VectorDataProducer1,
    VectorDataProducer2,
    IntVectorsToIntVector(
        "IntVectorsToIntVector",
        InputLocations=[
            str(VectorDataProducer1.OutputLocation),
            str(VectorDataProducer2.OutputLocation),
        ],
    ),
    FrExpTransformer("FrExpTransformer"),
    LdExpTransformer("LdExpTransfomer"),
    OptFrExpTransformer("OptFrExpTransformer"),
    OptLdExpTransformer("OptLdExpTransformer"),
    CountingConsumer("CountingConsumer"),
    SDataProducer1,
    SDataProducer2,
    SRangesToIntVector(
        "SRangesToIntVector",
        InputRanges=[
            str(SDataProducer1.OutputLocation),
            str(SDataProducer2.OutputLocation),
        ],
    ),
    IntVectorsMerger(
        "IntVectorsMerger",
        InputLocations=[
            str(VectorDataProducer1.OutputLocation),
            str(VectorDataProducer2.OutputLocation),
        ],
    ),
    IntVectorsMergingConsumer(
        "IntVectorsMergingConsumer",
        InputLocations=[
            str(VectorDataProducer1.OutputLocation),
            str(VectorDataProducer2.OutputLocation),
        ],
    ),
    TwoDMerger(
        "TwoDMerger",
        InputInts=[
            str(VectorDataProducer1.OutputLocation),
            str(VectorDataProducer2.OutputLocation),
        ],
        InputDoubles=[
            str(OptLdExpTransformer("OptLdExpTransformer").OutputDoubles),
        ],
        OutputInts="/Event/SummedInts",
        OutputDoubles="/Event/SummedDoubles",
    ),
    OpaqueProducer("OpaqueProducer"),
    ShrdPtrProducer("SharedPtrProducer"),
    ShrdPtrConsumer("SharedPtrConsumer"),
    VectorDataProducer(
        "IntProducer0", Data=[0, 2, 4], OutputLocation="/Event/EventInts"
    ),
    VectorDataProducer(
        "IntProducer1", Data=[1, 3, 5, 7], OutputLocation="/Event/OddInt"
    ),
    VectorDataProducer(
        "IntProducer2", Data=[1, 1, 2, 3, 5, 8, 13], OutputLocation="/Event/Fib7"
    ),
    IntVectorsToInts(
        "IntVectorsToInts",
        Mapping=[[0, 0], [0, 1], [0, 2], [1, 1], [1, 2]],
        InputLocations=["/Event/EventInts", "/Event/OddInt", "/Event/Fib7"],
        OutputLocations=["/Event/EvenOddFib", "/Event/OddFib"],
    ),
    ContextIntConsumer("EvenOddFibConsumer", InputLocation="/Event/EvenOddFib"),
    ContextIntConsumer("OddFibConsumer", InputLocation="/Event/OddFib"),
]
# - Events
app.EvtMax = 2
app.EvtSel = "NONE"
app.HistogramPersistency = "NONE"

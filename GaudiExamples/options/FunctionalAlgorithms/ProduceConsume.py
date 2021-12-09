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
from Configurables import Gaudi__Examples__ContextConsumer as ContextConsumer
from Configurables import Gaudi__Examples__ContextIntConsumer as ContextIntConsumer
from Configurables import Gaudi__Examples__CountingConsumer as CountingConsumer
from Configurables import Gaudi__Examples__FloatDataConsumer as FloatDataConsumer
from Configurables import Gaudi__Examples__FrExpTransformer as FrExpTransformer
from Configurables import Gaudi__Examples__IntDataConsumer as IntDataConsumer
from Configurables import Gaudi__Examples__IntDataProducer as IntDataProducer
from Configurables import (
    Gaudi__Examples__IntIntToFloatFloatData as IntIntToFloatFloatData,
)
from Configurables import Gaudi__Examples__IntToFloatData as IntToFloatData
from Configurables import Gaudi__Examples__IntVectorsMerger as IntVectorsMerger
from Configurables import (
    Gaudi__Examples__IntVectorsMergingConsumer as IntVectorsMergingConsumer,
)
from Configurables import (
    Gaudi__Examples__IntVectorsToIntVector as IntVectorsToIntVector,
)
from Configurables import Gaudi__Examples__LdExpTransformer as LdExpTransformer
from Configurables import Gaudi__Examples__MyConsumerTool as MyConsumerTool
from Configurables import Gaudi__Examples__MyExampleTool as MyExampleTool
from Configurables import Gaudi__Examples__OptFrExpTransformer as OptFrExpTransformer
from Configurables import Gaudi__Examples__OptLdExpTransformer as OptLdExpTransformer
from Configurables import Gaudi__Examples__SDataProducer as SDataProducer
from Configurables import Gaudi__Examples__SRangesToIntVector as SRangesToIntVector
from Configurables import Gaudi__Examples__ToolConsumer as ToolConsumer
from Configurables import Gaudi__Examples__VectorDataProducer as VectorDataProducer
from Configurables import Gaudi__Examples__VectorDoubleProducer as VectorDoubleProducer
from Configurables import Gaudi__Monitoring__MessageSvcSink as MessageSvcSink
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
]
# - Events
app.EvtMax = 2
app.EvtSel = "NONE"
app.HistogramPersistency = "NONE"

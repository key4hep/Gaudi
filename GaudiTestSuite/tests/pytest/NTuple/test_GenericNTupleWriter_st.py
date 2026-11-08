#####################################################################################
# (c) Copyright 2024-2026 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import os
import sys

import pytest

sys.path.append(os.path.dirname(__file__))

from NTupleWriterTestBase import (
    EXPECTED_ENTRIES,
    EXPECTED_STRING_VALUE,
    OUTPUT_FILE_NAME,
    RNTUPLE_ELEMENT_NAME,
    RNTUPLE_NAME,
    TTREE_ELEMENT_NAME,
    TTREE_NAME,
    NTupleWriterTestBase,
)


@pytest.mark.ctest_fixture_setup("ntuple_io_st")
@pytest.mark.shared_cwd("ntuple_io_st")
class TestST(NTupleWriterTestBase):
    command = ["gaudirun.py", f"{__file__}:config_st"]


def config_st():
    """
    Configuration function for the Gaudi application. Sets up components, services, and producers
    """
    from Configurables import (
        ApplicationMgr,
        FileSvc,
        Gaudi__NTuple__GenericRNTupleElementWriter,
        Gaudi__NTuple__GenericRNTupleWriter,
        Gaudi__NTuple__GenericTTreeElementWriter,
        Gaudi__NTuple__GenericWriter,
        Gaudi__TestSuite__NTuple__CounterDataProducer,
        Gaudi__TestSuite__NTuple__FloatDataProducer,
        Gaudi__TestSuite__NTuple__FloatVectorProducer,
        Gaudi__TestSuite__NTuple__IntVectorDataProducer,
        Gaudi__TestSuite__NTuple__StrDataProducer,
        Gaudi__TestSuite__NTuple__StructDataProducer,
        Gaudi__TestSuite__NTuple__StructVectorProducer,
        IncidentSvc,
        MessageSvc,
    )
    from Configurables import Gaudi__RootCnvSvc as RootCnvSvc

    from Gaudi.Configuration import DEBUG, INFO

    # Output Levels
    MessageSvc(OutputLevel=INFO)
    IncidentSvc(OutputLevel=INFO)
    RootCnvSvc(OutputLevel=INFO)

    # Create producers (float/std::vector/std::string/MyStruct)
    producers = [
        Gaudi__TestSuite__NTuple__FloatDataProducer("FProducer", OutputLevel=DEBUG),
        Gaudi__TestSuite__NTuple__IntVectorDataProducer("VProducer", OutputLevel=DEBUG),
        Gaudi__TestSuite__NTuple__StrDataProducer(
            "SProducer", OutputLevel=DEBUG, StringValue=EXPECTED_STRING_VALUE
        ),
        Gaudi__TestSuite__NTuple__StructDataProducer("STProducer", OutputLevel=DEBUG),
        Gaudi__TestSuite__NTuple__CounterDataProducer("CounterDataProducer"),
    ]
    # Vectors of the same length for the element writers
    vectorProducers = [
        Gaudi__TestSuite__NTuple__FloatVectorProducer("FVProducer", OutputLevel=DEBUG),
        Gaudi__TestSuite__NTuple__StructVectorProducer(
            "STVProducer", OutputLevel=DEBUG
        ),
    ]

    # Configure the NTuple writers, one entry per event
    eventInputs = dict(
        OutputLevel=DEBUG,
        OutputFile="NTuple",
        Inputs=[str(p.OutputLocation) for p in producers],
        FieldTypes=[p.OutputLocation.Type for p in producers],
    )
    # or one entry per vector element, with MyFloat repeated in every entry
    elementProducers = producers[:1] + vectorProducers
    elementInputs = dict(
        OutputLevel=DEBUG,
        OutputFile="NTuple",
        Inputs=[str(p.OutputLocation) for p in elementProducers],
        FieldTypes=[p.OutputLocation.Type for p in elementProducers],
        EventFields=["MyFloat"],
    )
    writers = [
        # GenericWriter is the former name of GenericTTreeWriter
        Gaudi__NTuple__GenericWriter(TTREE_NAME, **eventInputs),
        Gaudi__NTuple__GenericRNTupleWriter(RNTUPLE_NAME, **eventInputs),
        Gaudi__NTuple__GenericTTreeElementWriter(TTREE_ELEMENT_NAME, **elementInputs),
        Gaudi__NTuple__GenericRNTupleElementWriter(
            RNTUPLE_ELEMENT_NAME, **elementInputs
        ),
    ]

    fileSvc = FileSvc(
        Config={
            "NTuple": f"{OUTPUT_FILE_NAME}?mode=recreate",
        }
    )

    # Application setup
    ApplicationMgr(
        TopAlg=producers + vectorProducers + writers,
        EvtMax=EXPECTED_ENTRIES,
        EvtSel="NONE",
        HistogramPersistency="NONE",
        ExtSvc=[fileSvc],
    )

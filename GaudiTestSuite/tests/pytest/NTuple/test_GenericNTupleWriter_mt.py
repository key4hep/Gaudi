#####################################################################################
# (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      #
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

sys.path.append(os.path.dirname(__file__))

from NTpleWriterTestBase import (
    ALG_NAME,
    EXPECTED_ENTRIES,
    EXPECTED_STRING_VALUE,
    OUTPUT_FILE_NAME,
    NTpleWriterTestBase,
)


class TestMT(NTpleWriterTestBase):
    command = ["gaudirun.py", f"{__file__}:config_mt"]


def config_mt():
    """
    Configuration function for the Gaudi application. Sets up components, services, and producers
    """
    from Configurables import (
        AlgResourcePool,
        ApplicationMgr,
        AvalancheSchedulerSvc,
        FileSvc,
        Gaudi__NTuple__GenericWriter,
        Gaudi__TestSuite__NTuple__CounterDataProducer,
        Gaudi__TestSuite__NTuple__FloatDataProducer,
        Gaudi__TestSuite__NTuple__IntVectorDataProducer,
        Gaudi__TestSuite__NTuple__StrDataProducer,
        Gaudi__TestSuite__NTuple__StructDataProducer,
        HiveSlimEventLoopMgr,
        HiveWhiteBoard,
    )

    from Gaudi.Configuration import DEBUG, WARNING

    # Configuration parameters for the multithreaded environment
    evtslots = 4
    evtMax = EXPECTED_ENTRIES
    threads = 4

    # Whiteboard setup
    whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots)

    # Event Loop Manager
    slimeventloopmgr = HiveSlimEventLoopMgr(
        SchedulerName="AvalancheSchedulerSvc", OutputLevel=WARNING
    )

    # Scheduler
    AvalancheSchedulerSvc(ThreadPoolSize=threads, OutputLevel=WARNING)

    # Algorithm Resource Pool
    AlgResourcePool(OutputLevel=DEBUG)

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

    # NTupleWriter configuration
    NTupleWriter = Gaudi__NTuple__GenericWriter(
        ALG_NAME, OutputLevel=DEBUG, OutputFile="NTuple"
    )
    NTupleWriter.ExtraInputs = [
        ("float", "MyFloat"),
        ("std::vector<int>", "MyVector"),
        ("std::string", "MyString"),
        ("Gaudi::TestSuite::NTuple::MyStruct", "MyStruct"),
        ("Gaudi::TestSuite::Counter", "MyCounter"),
    ]
    # NTupleWriter.ExtraInputs = [
    #     (alg.Output.Type, str(alg.Output))
    #     for alg in producers
    # ]

    fileSvc = FileSvc(
        Config={
            "NTuple": f"{OUTPUT_FILE_NAME}?mode=create",
        }
    )

    # Application setup
    ApplicationMgr(
        EvtMax=evtMax,
        EvtSel="NONE",
        ExtSvc=[whiteboard, fileSvc],
        EventLoop=slimeventloopmgr,
        TopAlg=producers + [NTupleWriter],
        MessageSvcType="InertMessageSvc",
    )

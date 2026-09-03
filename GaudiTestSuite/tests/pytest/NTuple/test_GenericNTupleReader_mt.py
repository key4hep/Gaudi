#####################################################################################
# (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import os
import re
import sys

import pytest
from GaudiTesting import GaudiExeTest

sys.path.append(os.path.dirname(__file__))

from NTpleWriterTestBase import (
    ALG_NAME as WRITER_ALG_NAME,
)
from NTpleWriterTestBase import (
    EXPECTED_ENTRIES,
    EXPECTED_STRING_VALUE,
)
from NTpleWriterTestBase import (
    OUTPUT_FILE_NAME as INPUT_FILE_NAME,
)


@pytest.mark.ctest_fixture_required("ntuple_io_mt")
@pytest.mark.shared_cwd("ntuple_io_mt")
class TestMT(GaudiExeTest):
    command = ["gaudirun.py", f"{__file__}:config_mt"]

    def test_stdout(self, stdout: bytes):
        lines = stdout.decode("utf-8").splitlines()
        for t, v in [
            ("float", r"2\.5"),
            ("vector", r"\[0, 1, 2, 3, 4\]"),
            ("string", EXPECTED_STRING_VALUE),
            ("struct", r"MyStruct { id: 1, name: myStruct }"),
            ("counter", r"DataObject at 0x[0-9a-f]+"),
        ]:
            for expression in [f".+INFO +Received {t}: {v}$"]:
                assert (
                    sum(1 for line in lines if re.match(expression, line))
                    == EXPECTED_ENTRIES
                ), f"expected regex r'{expression}' missing"


def config_mt():
    """
    Configuration function for the Gaudi application. Sets up components, services, and consumers
    """
    from Configurables import (
        AlgResourcePool,
        ApplicationMgr,
        AvalancheSchedulerSvc,
        FileSvc,
        Gaudi__NTuple__GenericReader,
        Gaudi__TestSuite__NTuple__CounterDataConsumer,
        Gaudi__TestSuite__NTuple__FloatDataConsumer,
        Gaudi__TestSuite__NTuple__IntVectorDataConsumer,
        Gaudi__TestSuite__NTuple__StrDataConsumer,
        Gaudi__TestSuite__NTuple__StructDataConsumer,
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

    # Create consumers (float/std::vector/std::string/MyStruct)
    consumers = [
        Gaudi__TestSuite__NTuple__FloatDataConsumer("FConsumer", OutputLevel=DEBUG),
        Gaudi__TestSuite__NTuple__IntVectorDataConsumer("VConsumer", OutputLevel=DEBUG),
        Gaudi__TestSuite__NTuple__StrDataConsumer("SConsumer", OutputLevel=DEBUG),
        Gaudi__TestSuite__NTuple__StructDataConsumer("STConsumer", OutputLevel=DEBUG),
        Gaudi__TestSuite__NTuple__CounterDataConsumer("CounterDataConsumer"),
    ]

    # Configure the NTupleReader
    NTupleReader = Gaudi__NTuple__GenericReader(
        "ReaderAlg", OutputLevel=DEBUG, InputFile="NTuple", NTupleName=WRITER_ALG_NAME
    )
    NTupleReader.ExtraOutputs = [
        ("float", "MyFloat"),
        ("std::vector<int>", "MyVector"),
        ("std::string", "MyString"),
        ("Gaudi::TestSuite::NTuple::MyStruct", "MyStruct"),
        ("Gaudi::TestSuite::Counter", "MyCounter"),
    ]
    # NTupleReader.ExtraOutputs = [
    #     (alg.Input.Type, str(alg.Input))
    #     for alg in consumers
    # ]

    fileSvc = FileSvc(
        Config={
            "NTuple": f"{INPUT_FILE_NAME}?mode=read",
        }
    )

    # Application setup
    ApplicationMgr(
        TopAlg=[NTupleReader] + consumers,
        EvtMax=evtMax,
        EvtSel="NONE",
        ExtSvc=[whiteboard, fileSvc],
        EventLoop=slimeventloopmgr,
        MessageSvcType="InertMessageSvc",
    )

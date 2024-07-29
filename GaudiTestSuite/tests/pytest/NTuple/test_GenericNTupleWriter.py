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
from contextlib import contextmanager

import pytest
import ROOT
from GaudiTests import run_gaudi

# Constants for the output file name and expected values for verification
OUTPUT_FILE_NAME = "generic_ntuple_writer_tree.root"
EXPECTED_ENTRIES = 10
EXPECTED_FLOAT_VALUE = 2.5
EXPECTED_VECTOR_CONTENT = [0, 1, 2, 3, 4]
EXPECTED_STRING_VALUE = "hello world"
NUM_PRODUCERS = 1


@contextmanager
def temp_chdir(path):
    curdir = os.getcwd()
    os.chdir(path)
    try:
        yield
    finally:
        os.chdir(curdir)


@pytest.fixture(scope="module", params=["st", "mt"])
def setup_file_tree(tmp_path_factory, request):
    """
    Fixture to set up the ROOT file and tree for testing with the given configuration.
    Args:
        tmp_path_factory: Factory for creating temporary directories provided by pytest.
        request: Pytest request object, which contains a parameter for the configuration to use.
    Yields:
        Tuple of (ROOT file, ROOT TTree) for use in tests.
    """
    with temp_chdir(tmp_path_factory.mktemp(request.param)):
        if os.path.exists(OUTPUT_FILE_NAME):
            os.remove(OUTPUT_FILE_NAME)
        run_gaudi(f"{__file__}:config_{request.param}", check=True)
        file = ROOT.TFile.Open(OUTPUT_FILE_NAME)
        tree = file.Get("GenericWriterTree")
    yield file, tree
    file.Close()


def config_st():
    """
    Configuration function for the Gaudi application. Sets up components, services, and producers
    """
    from Configurables import (
        ApplicationMgr,
        FileSvc,
        Gaudi__NTuple__GenericWriter,
        Gaudi__TestSuite__NTuple__FloatDataProducer,
        Gaudi__TestSuite__NTuple__IntVectorDataProducer,
        Gaudi__TestSuite__NTuple__StrDataProducer,
        Gaudi__TestSuite__NTuple__StructDataProducer,
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
    ]

    # Configure the NTupleWriter
    NTupleWriter = Gaudi__NTuple__GenericWriter(
        "NTupleWriter", OutputLevel=DEBUG, OutputFile="NTuple"
    )
    NTupleWriter.ExtraInputs = [
        ("float", "MyFloat"),
        ("std::vector<int>", "MyVector"),
        ("std::string", "MyString"),
        ("Gaudi::TestSuite::NTuple::MyStruct", "MyStruct"),
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
        TopAlg=producers + [NTupleWriter],
        EvtMax=EXPECTED_ENTRIES,
        EvtSel="NONE",
        HistogramPersistency="NONE",
        ExtSvc=[fileSvc],
    )


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
    ]

    # NTupleWriter configuration
    NTupleWriter = Gaudi__NTuple__GenericWriter(
        "NTupleWriter", OutputLevel=DEBUG, OutputFile="NTuple"
    )
    NTupleWriter.ExtraInputs = [
        ("float", "MyFloat"),
        ("std::vector<int>", "MyVector"),
        ("std::string", "MyString"),
        ("Gaudi::TestSuite::NTuple::MyStruct", "MyStruct"),
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


def test_file_creation_and_tree_structure(setup_file_tree):
    """
    Tests if the output ROOT file is created successfully and contains a tree
    """
    file, tree = setup_file_tree
    assert file, f"expected output file {OUTPUT_FILE_NAME} not found"
    assert tree, "TTree 'NTupleWriterTree' not found in the file"


def test_branch_creation(setup_file_tree):
    """
    Verifies that the expected branches are created in the ROOT file
    """
    _, tree = setup_file_tree
    assert tree.GetBranch("MyFloat") is not None, "Float branch was not created."
    assert tree.GetBranch("MyVector") is not None, "Vector<int> branch was not created."
    assert tree.GetBranch("MyString") is not None, "String branch was not created."
    assert tree.GetBranch("MyStruct") is not None, "MyStruct branch was not created."


def test_float_branch_content(setup_file_tree):
    """
    Tests the content of the float branch to ensure it matches the expected value
    """
    _, tree = setup_file_tree
    for entry in tree:
        assert (
            entry.MyFloat == EXPECTED_FLOAT_VALUE
        ), "Float branch does not contain the correct value."


def test_string_branch_content(setup_file_tree):
    """
    Tests the content of the string branch to ensure it matches the expected value
    """
    _, tree = setup_file_tree
    for entry in tree:
        assert (
            entry.MyString == EXPECTED_STRING_VALUE
        ), "String branch does not contain the correct value."


def test_vector_branch_content(setup_file_tree):
    """
    Tests the content of the std::vector<int> branch to ensure it matches the expected value
    """
    _, tree = setup_file_tree
    for entry in tree:
        assert (
            list(entry.MyVector) == EXPECTED_VECTOR_CONTENT
        ), "Vector<int> branch does not contain the correct values."


def test_multiple_entries(setup_file_tree):
    """
    Basic check to ensure data for all expected events is present in the tree
    """
    _, tree = setup_file_tree
    assert (
        tree.GetEntries() == EXPECTED_ENTRIES
    ), f"Expected {EXPECTED_ENTRIES} entries, found {tree.GetEntries()}."


def test_handling_missing_input(setup_file_tree):
    """
    Checks how the algorithm handles a scenario where expected input data is missing
    """
    _, tree = setup_file_tree
    try:
        missing_branch = tree.GetBranch("MissingData")
        # Attempt to access a null pointer to trigger a ReferenceError
        missing_branch.GetName()
        assert False, "Branch 'MissingData' unexpectedly exists."
    except ReferenceError:
        assert True

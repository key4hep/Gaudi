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

import pytest
import ROOT
from GaudiTests import run_gaudi

# Constants for the output file name
OUTPUT_FILE_NAME = "ntuple_writer_tree.root"
EXPECTED_ENTRIES = 10
EXPECTED_VECTOR_SUM = 10
EXPECTED_VECTOR_SIZE = 5


@pytest.fixture(scope="module")
def setup_file_tree(tmp_path_factory):
    """
    PyTest fixture that prepares a ROOT file and tree for testing
    Args:
        tmp_path_factory: Factory for creating temporary directories provided by pytest
    Yields:
        Tuple of (ROOT file, ROOT TTree) for use in tests
    """
    os.chdir(tmp_path_factory.getbasetemp())
    if os.path.exists(OUTPUT_FILE_NAME):
        os.remove(OUTPUT_FILE_NAME)
    run_gaudi(f"{__file__}:config", check=True)
    file = ROOT.TFile.Open(OUTPUT_FILE_NAME)
    tree = file.Get("WriterTree")
    yield file, tree
    file.Close()


def config():
    """
    Configuration function for the Gaudi application that sets up the NTupleWriter.
    """
    from GaudiConfig2 import Configurables as C

    E = C.Gaudi

    algs = [
        E.TestSuite.NTuple.IntVectorDataProducer("IntVectorDataProducer"),
        E.TestSuite.NTuple.NTupleWriter_V(
            "NTupleWriter_V",
            TreeFilename=OUTPUT_FILE_NAME,
            BranchNames=["Branch1", "Branch2", "Branch3"],
        ),
    ]

    loopmgr = C.HiveSlimEventLoopMgr(SchedulerName="AvalancheSchedulerSvc")
    whiteboard = C.HiveWhiteBoard("EventDataSvc", EventSlots=5)
    svcs = [whiteboard, C.AlgResourcePool()]
    return (
        [
            C.ApplicationMgr(
                TopAlg=algs,
                EvtMax=EXPECTED_ENTRIES,
                EvtSel="NONE",
                ExtSvc=svcs,
                EventLoop=loopmgr.name,
            ),
            loopmgr,
        ]
        + algs
        + svcs
    )


def test_branch_creation(setup_file_tree):
    """
    Test to ensure that all expected branches are correctly created in the ROOT file.
    """
    _, tree = setup_file_tree
    assert tree.GetBranch("Branch1"), "Branch1 should exist in WriterTree."
    assert tree.GetBranch("Branch2"), "Branch2 should exist in WriterTree."
    assert tree.GetBranch("Branch3"), "Branch3 should exist in WriterTree."


def test_data_types(setup_file_tree):
    """
    Verify the data within the branches to ensure they match expected transformations.
    """
    _, tree = setup_file_tree
    for entry in tree:
        assert isinstance(
            entry.Branch1, int
        ), "Branch1 does not contain int values as expected."
        assert isinstance(
            entry.Branch2, int
        ), "Branch2 does not contain int values as expected."
        assert isinstance(
            entry.Branch3, float
        ), "Branch3 does not contain float values as expected."


def test_data_values(setup_file_tree):
    """ """
    _, tree = setup_file_tree
    for entry in tree:
        assert (
            entry.Branch1 == EXPECTED_VECTOR_SUM
        ), "Branch1 does not contain the correct value."
        assert (
            entry.Branch2 == EXPECTED_VECTOR_SIZE
        ), "Branch2 does not contain the correct value."

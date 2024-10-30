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
import pytest
import ROOT
from GaudiTesting import GaudiExeTest

# Constants for the output file name
OUTPUT_FILE_NAME = "ntuple_writer_tree.root"
EXPECTED_ENTRIES = 10
EXPECTED_VECTOR_SUM = 10
EXPECTED_VECTOR_SIZE = 5
ALG_NAME = "WriterAlg"


class Test(GaudiExeTest):
    command = ["gaudirun.py", f"{__file__}:config"]

    @pytest.fixture(scope="class")
    def setup_file_tree(self, cwd):
        """
        Fixture to yield the ROOT file and tree for testing with the given configuration.
        Args:
            cwd: Fixture returning the execution directory for the test.
        Yields:
            Tuple of the produced (ROOT file, ROOT TTree).
        """
        file = ROOT.TFile.Open(str(cwd / OUTPUT_FILE_NAME))
        tree = file.Get(ALG_NAME)
        yield file, tree
        file.Close()

    def test_branch_creation(self, setup_file_tree):
        """
        Test to ensure that all expected branches are correctly created in the ROOT file.
        """
        _, tree = setup_file_tree
        assert tree.GetBranch("Branch1"), "Branch1 should exist in WriterTree."
        assert tree.GetBranch("Branch2"), "Branch2 should exist in WriterTree."
        assert tree.GetBranch("Branch3"), "Branch3 should exist in WriterTree."

    def test_data_types(self, setup_file_tree):
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

    def test_data_values(self, setup_file_tree):
        """ """
        _, tree = setup_file_tree
        for entry in tree:
            assert (
                entry.Branch1 == EXPECTED_VECTOR_SUM
            ), "Branch1 does not contain the correct value."
            assert (
                entry.Branch2 == EXPECTED_VECTOR_SIZE
            ), "Branch2 does not contain the correct value."


def config():
    """
    Configuration function for the Gaudi application that sets up the NTupleWriter.
    """
    from GaudiConfig2 import Configurables as C

    E = C.Gaudi

    algs = [
        E.TestSuite.NTuple.IntVectorDataProducer("IntVectorDataProducer"),
        E.TestSuite.NTuple.NTupleWriter_V(
            ALG_NAME,
            OutputFile="NTuple",
            BranchNames=["Branch1", "Branch2", "Branch3"],
        ),
    ]

    fileSvc = C.FileSvc(
        Config={
            "NTuple": f"{OUTPUT_FILE_NAME}?mode=recreate",
        }
    )

    loopmgr = C.HiveSlimEventLoopMgr(SchedulerName="AvalancheSchedulerSvc")
    whiteboard = C.HiveWhiteBoard("EventDataSvc", EventSlots=5)
    svcs = [fileSvc, whiteboard, C.AlgResourcePool()]
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
        + [fileSvc]
    )

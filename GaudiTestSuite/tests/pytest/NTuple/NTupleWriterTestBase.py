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
import pytest
import ROOT
from GaudiTesting import GaudiExeTest

# Constants for the output file name and expected values for verification
OUTPUT_FILE_NAME = "generic_ntuple_writer_tree.root"
EXPECTED_ENTRIES = 100
EXPECTED_FLOAT_VALUE = 2.5
EXPECTED_VECTOR_CONTENT = [0, 1, 2, 3, 4]
EXPECTED_STRING_VALUE = "hello world"
# The vector producers give evt % 9 elements for event evt
EXPECTED_ELEMENTS = sum(evt % 9 for evt in range(EXPECTED_ENTRIES))

# One ntuple per writer, named after it
TTREE_NAME = "TTreeWriter"
RNTUPLE_NAME = "RNTupleWriter"
TTREE_ELEMENT_NAME = "TTreeElementWriter"
RNTUPLE_ELEMENT_NAME = "RNTupleElementWriter"


class NTupleWriterTestBase(GaudiExeTest):
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
        tree = file.Get(TTREE_NAME)
        yield file, tree
        file.Close()

    def test_file_creation_and_tree_structure(self, setup_file_tree):
        """
        Tests if the output ROOT file is created successfully and contains a tree
        """
        file, tree = setup_file_tree
        assert file, f"expected output file {OUTPUT_FILE_NAME} not found"
        assert tree, "TTree 'GenericWriterTree' not found in the file"

    def test_branch_creation(self, setup_file_tree):
        """
        Verifies that the expected branches are created in the ROOT file
        """
        _, tree = setup_file_tree
        assert tree.GetBranch("MyFloat") is not None, "Float branch was not created."
        assert (
            tree.GetBranch("MyVector") is not None
        ), "Vector<int> branch was not created."
        assert tree.GetBranch("MyString") is not None, "String branch was not created."
        assert (
            tree.GetBranch("MyStruct") is not None
        ), "MyStruct branch was not created."
        assert (
            tree.GetBranch("MyCounter") is not None
        ), "MyCounter branch was not created."

    def test_float_branch_content(self, setup_file_tree):
        """
        Tests the content of the float branch to ensure it matches the expected value
        """
        _, tree = setup_file_tree
        for entry in tree:
            assert (
                entry.MyFloat == EXPECTED_FLOAT_VALUE
            ), "Float branch does not contain the correct value."

    def test_string_branch_content(self, setup_file_tree):
        """
        Tests the content of the string branch to ensure it matches the expected value
        """
        _, tree = setup_file_tree
        for entry in tree:
            assert (
                entry.MyString == EXPECTED_STRING_VALUE
            ), "String branch does not contain the correct value."

    def test_vector_branch_content(self, setup_file_tree):
        """
        Tests the content of the std::vector<int> branch to ensure it matches the expected value
        """
        _, tree = setup_file_tree
        for entry in tree:
            assert (
                list(entry.MyVector) == EXPECTED_VECTOR_CONTENT
            ), "Vector<int> branch does not contain the correct values."

    def test_multiple_entries(self, setup_file_tree):
        """
        Basic check to ensure data for all expected events is present in the tree
        """
        _, tree = setup_file_tree
        assert (
            tree.GetEntries() == EXPECTED_ENTRIES
        ), f"Expected {EXPECTED_ENTRIES} entries, found {tree.GetEntries()}."

    def test_handling_missing_input(self, setup_file_tree):
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

    def test_rntuple_content(self, setup_file_tree):
        """
        Tests the RNTuple, written from the same inputs as the TTree
        """
        file, _ = setup_file_tree
        reader = ROOT.RNTupleReader.Open(file.Get(RNTUPLE_NAME))
        assert (
            reader.GetNEntries() == EXPECTED_ENTRIES
        ), f"Expected {EXPECTED_ENTRIES} entries, found {reader.GetNEntries()}."
        entry = reader.CreateEntry()
        for i in range(reader.GetNEntries()):
            reader.LoadEntry(i, entry)
            assert (
                entry["MyFloat"] == EXPECTED_FLOAT_VALUE
            ), "Float field does not contain the correct value."
            assert (
                entry["MyString"] == EXPECTED_STRING_VALUE
            ), "String field does not contain the correct value."
            assert (
                list(entry["MyVector"]) == EXPECTED_VECTOR_CONTENT
            ), "Vector<int> field does not contain the correct values."

    def test_element_entries(self, setup_file_tree):
        """
        Checks that the element writers write one entry per vector element
        """
        file, _ = setup_file_tree
        tree = file.Get(TTREE_ELEMENT_NAME)
        reader = ROOT.RNTupleReader.Open(file.Get(RNTUPLE_ELEMENT_NAME))
        assert (
            tree.GetEntries() == EXPECTED_ELEMENTS
        ), f"Expected {EXPECTED_ELEMENTS} TTree entries, found {tree.GetEntries()}."
        assert (
            reader.GetNEntries() == EXPECTED_ELEMENTS
        ), f"Expected {EXPECTED_ELEMENTS} RNTuple entries, found {reader.GetNEntries()}."

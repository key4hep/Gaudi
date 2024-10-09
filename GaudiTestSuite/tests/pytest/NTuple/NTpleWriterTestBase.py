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

# Constants for the output file name and expected values for verification
OUTPUT_FILE_NAME = "generic_ntuple_writer_tree.root"
EXPECTED_ENTRIES = 100
EXPECTED_FLOAT_VALUE = 2.5
EXPECTED_VECTOR_CONTENT = [0, 1, 2, 3, 4]
EXPECTED_STRING_VALUE = "hello world"
ALG_NAME = "WriterAlg"


class NTpleWriterTestBase(GaudiExeTest):
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

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
import subprocess
from pathlib import Path
from unittest.mock import MagicMock, patch

import pytest
from GaudiTesting import GaudiExeTest, platform_matches
from GaudiTesting.SubprocessBaseTest import SubprocessBaseTest


def test_resolve_path():
    class TestResolvePath(SubprocessBaseTest):
        pass

    test_path = Path(__file__).name
    resolved_path = TestResolvePath().resolve_path(test_path)
    assert os.path.isabs(resolved_path), "Path should be resolved to an absolute path"
    assert resolved_path == __file__, "Path should be resolved relative to this file"

    text = "arbitrary_non_path"
    resolved_text = TestResolvePath().resolve_path("arbitrary_non_path")
    assert text == resolved_text, "Arbitrary text should not be modified"


def test_update_env():
    class TestUpdateEnv(SubprocessBaseTest):
        environment = ["TEST_KEY=test_value"]

    env = {}
    TestUpdateEnv.update_env(env)
    assert env["TEST_KEY"] == "test_value", "Environment variable should be updated"


def test_prepare_command():
    class TestPrepareCommand(SubprocessBaseTest):
        command = ["echo", "/Hello"]

    with patch("GaudiTesting.SubprocessBaseTest.which", return_value="/bin/echo"):
        prepared_command = TestPrepareCommand()._prepare_command()
        expected_command = ["/bin/echo", "/Hello"]
        assert (
            prepared_command == expected_command
        ), f"Expected {expected_command}, but got {prepared_command}"


def test_handle_timeout():
    mock_proc = MagicMock()

    class TestHandleTimeout(SubprocessBaseTest):
        pass

    with patch.object(
        TestHandleTimeout, "_collect_stack_trace", return_value="stack trace"
    ) as mock_collect_stack_trace, patch.object(
        TestHandleTimeout, "_terminate_process"
    ) as mock_terminate_process:
        stack_trace = TestHandleTimeout()._handle_timeout(mock_proc)
        assert (
            stack_trace == "stack trace"
        ), f"Expected 'stack trace', but got {stack_trace}"
        mock_collect_stack_trace.assert_called_once_with(mock_proc)
        mock_terminate_process.assert_called_once_with(mock_proc)


def test_output_diff():
    reference = "expected output"
    output = "actual output"
    diff = GaudiExeTest._output_diff(reference, output)
    assert diff


def test_test_stderr(record_property):
    class TestStderr(GaudiExeTest):
        reference = "/path/to/reference"

    completed_process = subprocess.CompletedProcess(
        args=[], returncode=0, stdout=b"", stderr=b"test error"
    )

    TestStderr().test_stderr(
        completed_process.stderr, record_property, {"stderr": "test error"}
    )


def test_test_returncode():
    class TestReturncode(GaudiExeTest):
        pass

    completed_process = subprocess.CompletedProcess(args=[], returncode=1)
    with pytest.raises(AssertionError):
        TestReturncode().test_returncode(completed_process.returncode)


def test_find_reference_block(record_property):
    class TestFindReferenceBlock(GaudiExeTest):
        pass

    reference_block = """
    expected line 1
    expected line 2
    expected line 3
    """

    stdout1 = "expected line 0\nexpected line 1\nexpected line 2\nexpected line 3\n"
    stdout2 = "expected line 0\nexpected line 1\nUNexpected line 2\n\n"
    assert_function = TestFindReferenceBlock.find_reference_block(
        reference_block, lambda x: x
    )

    assert_function(TestFindReferenceBlock, stdout1.encode("utf-8"), record_property)

    with pytest.raises(AssertionError):
        assert_function(
            TestFindReferenceBlock, stdout2.encode("utf-8"), record_property
        )


def test_count_error_lines():
    stdout = (
        "1 ERROR something bad\n2 FATAL something very bad\n3 ERROR another bad thing\n"
    )
    expected = {"ERROR": 2, "FATAL": 1}

    GaudiExeTest.count_error_lines(expected, stdout)


def test_skip_if_platform_matches():
    with patch.dict("os.environ", {"BINARY_TAG": "x86_64-centos7-gcc8-opt"}):
        assert platform_matches(["gcc8-opt"])
        assert not platform_matches(["i686", "arm"])

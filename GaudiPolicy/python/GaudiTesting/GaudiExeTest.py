#####################################################################################
# (c) Copyright 2024-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import difflib
import inspect
import json
import os
import re
import time
from pathlib import Path
from textwrap import dedent
from typing import Callable, Dict, List

import pytest

from GaudiTesting.preprocessors import normalizeTestSuite
from GaudiTesting.SubprocessBaseTest import SubprocessBaseTest
from GaudiTesting.utils import (
    CodeWrapper,
    filter_dict,
    find_histos_summaries,
    find_ttree_summaries,
)

NO_ERROR_MESSAGES = {"ERROR": 0, "FATAL": 0}


class GaudiExeTest(SubprocessBaseTest):
    """
    An extension of SubprocessBaseTest tailored to the Gaudi/LHCb workflow.
    It includes additional functionalities for handling options,
    preprocessing output, and validating against platform-specific reference files.
    """

    options: Callable = None
    options_code = None
    preprocessor: Callable = normalizeTestSuite

    @classmethod
    def _prepare_command(cls, tmp_path=Path()) -> List[str]:
        """
        Override the base class to include options.
        """
        command = super()._prepare_command(tmp_path=tmp_path)

        def generate_unique_options_filename(extension, directory):
            timestamp = time.strftime("%Y%m%d_%H%M%S")
            return directory / f"options_{timestamp}.{extension}"

        if hasattr(cls, "options") and cls.options is not None:
            options = cls.options
            filename = None

            # Check if options is a function
            if callable(options):
                source_lines = inspect.getsource(options).splitlines()
                clean_source = dedent(
                    "\n".join(source_lines[1:])
                )  # Skip the first line (def options():)
                filename = generate_unique_options_filename("py", tmp_path)
                with open(filename, "w") as file:
                    file.write(clean_source)
                    cls.options_code = CodeWrapper(clean_source, "python")

            # Check if options is a dictionary
            elif isinstance(options, dict):
                filename = generate_unique_options_filename("json", tmp_path)
                with open(filename, "w") as file:
                    json.dump(options, file, indent=4)
                    cls.options_code = CodeWrapper(options, "json")

            # Check if options is a string
            elif isinstance(options, str):
                options = dedent(options)
                filename = generate_unique_options_filename("opts", tmp_path)
                with open(filename, "w") as file:
                    file.write(options)
                    cls.options_code = CodeWrapper(options, "cpp")

            else:
                raise ValueError(f"invalid options type '{type(options).__name__}'")

            if filename:
                command.append(str(filename))
        return command

    @staticmethod
    def _output_diff(
        reference_data: dict,
        output: str,
        preprocessor: Callable[[str], str] = lambda x: x,
    ) -> str:
        """
        Compute the difference between the reference data and the current output.
        """
        expected_output = (
            reference_data.splitlines()
            if hasattr(reference_data, "splitlines")
            else reference_data
        )
        actual_output = preprocessor(output).splitlines()
        return "\n".join(
            difflib.unified_diff(
                expected_output,
                actual_output,
                fromfile="expected",
                tofile="actual",
                lineterm="",
            )
        )

    @classmethod
    def validate_with_reference(
        cls,
        data: bytes,
        key: str,
        reference: Dict,
        record_property: Callable[[str, str], None],
    ) -> None:
        """
        Validate the given data against a reference file for the specified key.
        """
        if cls.reference:
            try:
                if key in reference:
                    # If the data is less than 100k, we can compare it directly
                    # and let pytest generate a nice diff, otherwise we hide the values
                    # to pytest to avoid that the test (practically) hangs.
                    #
                    # See
                    # - https://gitlab.cern.ch/lhcb/LHCb/-/issues/252
                    # - https://gitlab.cern.ch/gaudi/Gaudi/-/merge_requests/1375
                    # - https://gitlab.cern.ch/lhcb/LHCb/-/merge_requests/4773#note_9075367
                    if len(data) < 100000:
                        assert data == reference[key]
                    else:
                        same_as_reference = data == reference[key]
                        assert same_as_reference, "data is different from reference"
            except AssertionError:
                record_property(
                    f"{key}_diff",
                    CodeWrapper(
                        cls._output_diff(reference[key] or "", data, cls.preprocessor),
                        "diff",
                    ),
                )
                reference[key] = data
                if os.environ.get("GAUDI_TEST_IGNORE_STDOUT_VALIDATION") == "1":
                    pytest.xfail("Ignoring stdout validation")
                raise
        else:
            pytest.skip("No reference file provided")

    @classmethod
    def validate_json_with_reference(
        cls, output_file: str, reference_file: str, detailed=True
    ):
        """
        Validate the JSON output against a reference JSON file.
        """
        assert os.path.isfile(output_file)

        try:
            with open(output_file) as f:
                output = json.load(f)
        except json.JSONDecodeError as err:
            pytest.fail(f"json parser error in {output_file}: {err}")

        lreference = cls.resolve_path(reference_file)
        assert lreference, "reference file not set"
        assert os.path.isfile(lreference)

        try:
            with open(lreference) as f:
                expected = json.load(f)
        except json.JSONDecodeError as err:
            pytest.fail(f"JSON parser error in {lreference}: {err}")

        if not detailed:
            assert output == expected

        expected = sorted(expected, key=lambda item: (item["component"], item["name"]))
        output = sorted(output, key=lambda item: (item["component"], item["name"]))
        assert output == expected

    @classmethod
    def find_reference_block(
        cls,
        reference_block: str,
        preprocessor: Callable = None,
        signature: str = None,
        signature_offset: int = 0,
    ):
        def assert_function(
            cls,
            stdout,
            record_property,
            preprocessor=preprocessor,
            signature=signature,
            signature_offset=signature_offset,
        ):
            processed_stdout = (
                preprocessor(stdout.decode("utf-8"))
                if preprocessor
                else stdout.decode("utf-8")
            )
            stdout_lines = processed_stdout.strip().split("\n")
            reference_lines = dedent(reference_block).strip().split("\n")

            if signature is None and signature_offset is not None:
                if signature_offset < 0:
                    signature_offset = len(reference_lines) + signature_offset
                signature = reference_lines[signature_offset]

            try:
                start_index = stdout_lines.index(signature)
                end_index = start_index + len(reference_lines)
                observed_block = stdout_lines[start_index:end_index]

                if observed_block != reference_lines:
                    diff = list(
                        difflib.unified_diff(
                            reference_lines,
                            observed_block,
                            fromfile="expected",
                            tofile="actual",
                        )
                    )
                    diff_text = "\n".join(diff)
                    record_property("block_diff", CodeWrapper(diff_text, "diff"))
                    raise AssertionError(
                        "The observed block does not match the reference."
                    )
            except ValueError:
                raise AssertionError(
                    f"Signature '{signature}' not found in the output."
                )

        return assert_function

    @pytest.mark.do_not_collect_source
    def test_count_messages(self, reference, stdout, record_property):
        """
        Test the count of error messages in the stdout against expected values.
        """
        expected_messages = (
            None if reference is None else reference.get("messages_count")
        )
        if expected_messages is None:
            pytest.skip()

        if not isinstance(expected_messages, dict):
            raise ValueError("reference['messages_count'] must be a dict")
        if not expected_messages:
            # an empty dict doesn't make sense, let's assume we are bootstrapping the reference
            # and start from the default
            expected_messages = NO_ERROR_MESSAGES
            reference["messages_count"] = expected_messages

        outlines = self.preprocessor(
            stdout.decode("utf-8", errors="backslashreplace")
        ).splitlines()

        messages = {key: [] for key in expected_messages}
        for n, line in enumerate(outlines, 1):
            words = line.split()
            if len(words) >= 2 and words[1] in messages:
                messages[words[1]].append((n, line.rstrip()))

        messages_count = {key: len(value) for key, value in messages.items()}
        try:
            assert messages_count == expected_messages
        except AssertionError:
            reference["messages_count"] = messages_count
            record_property("unexpected_messages_count", messages)
            raise

    @pytest.mark.do_not_collect_source
    def test_stdout(
        self, stdout: bytes, record_property: Callable, reference: Dict
    ) -> None:
        """
        Test the standard output against the reference.
        """
        out = self.preprocessor(stdout.decode("utf-8", errors="backslashreplace"))
        self.validate_with_reference(out, "stdout", reference, record_property)

    @pytest.mark.do_not_collect_source
    def test_ttrees(
        self, stdout: bytes, record_property: Callable, reference: Dict
    ) -> None:
        """
        Test the TTree summaries against the reference.
        """
        if not self.reference or reference.get("ttrees") is None:
            pytest.skip()

        ttrees = filter_dict(
            find_ttree_summaries(stdout.decode()),
            re.compile(r"Basket|.*size|Compression"),
        )
        try:
            assert ttrees == reference["ttrees"]
        except AssertionError:
            reference["ttrees"] = ttrees
            if os.environ.get("GAUDI_TEST_IGNORE_STDOUT_VALIDATION") == "1":
                pytest.xfail("Ignoring stdout validation")
            raise

    @pytest.mark.do_not_collect_source
    def test_histos(
        self, stdout: bytes, record_property: Callable, reference: Dict
    ) -> None:
        """
        Test the histogram summaries against the reference.
        """
        if not self.reference or reference.get("histos") is None:
            pytest.skip()

        histos = filter_dict(
            find_histos_summaries(stdout.decode()),
            re.compile(r"Basket|.*size|Compression"),
        )
        try:
            assert histos == reference["histos"]
        except AssertionError:
            reference["histos"] = histos
            if os.environ.get("GAUDI_TEST_IGNORE_STDOUT_VALIDATION") == "1":
                pytest.xfail("Ignoring stdout validation")
            raise

    @pytest.mark.do_not_collect_source
    def test_stderr(
        self, stderr: bytes, record_property: Callable, reference: Dict
    ) -> None:
        """
        Test the standard error output against the reference.
        """
        err = self.preprocessor(stderr.decode("utf-8", errors="backslashreplace"))
        if self.reference and reference.get("stderr") is not None:
            self.validate_with_reference(err, "stderr", reference, record_property)

        else:
            assert not err.strip(), "Expected no standard error output, but got some."

    @pytest.mark.do_not_collect_source
    def test_record_options(self, record_property: Callable):
        if self.options_code:
            record_property("options", self.options_code)

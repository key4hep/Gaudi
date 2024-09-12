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
import inspect
import os
import subprocess
from collections import defaultdict
from pathlib import Path
from typing import Callable, Generator, Optional

import pytest
import yaml
from GaudiTesting.FixtureResult import FixtureResult
from GaudiTesting.pytest.ctest_measurements_reporter import results
from GaudiTesting.SubprocessBaseTest import SubprocessBaseTest
from GaudiTesting.utils import (
    CodeWrapper,
    expand_reference_file_name,
    file_path_for_class,
    get_platform,
    str_representer,
)


class AlwaysTrueDefaultDict(defaultdict):
    def __contains__(self, key):
        return True


yaml.representer.SafeRepresenter.add_representer(str, str_representer)
yaml.representer.SafeRepresenter.add_representer(
    AlwaysTrueDefaultDict, yaml.representer.SafeRepresenter.represent_dict
)

# This plugin provides a set of pytest fixtures for the Gaudi testing framework.
# These fixtures facilitate the execution of tests and capture various aspects
# of the test results.

# The fixtures include:
# - fixture_result: Runs the program and yields the result.
# - completed_process: Yields the completed subprocess process.
# - stdout: Captures the standard output of the subprocess.
# - stderr: Captures the standard error of the subprocess.
# - returncode: Captures the return code of the subprocess.
# - cwd: Captures the directory in which the program was executed.
# - check_for_exceptions: Skips the test if exceptions are found in the result.
# - capture_class_docstring: Captures the docstring of the test class.
# - reference: Creates a .new file if the output data is different from the reference.


def pytest_configure(config):
    config.addinivalue_line(
        "markers",
        "shared_cwd(id): make SubprocessBaseTest tests share a working directory",
    )
    config.addinivalue_line(
        "markers",
        "do_not_collect_source: flag the test code as not to be collected",
    )


def pytest_collection_modifyitems(config, items):
    """
    Record source code of tests.
    """
    for item in items:
        if isinstance(item, pytest.Function) and not item.get_closest_marker(
            "do_not_collect_source"
        ):
            name = (
                f"{item.parent.name}.{item.originalname}"
                if isinstance(item.parent, pytest.Class)
                else item.originalname
            )
            source_code = CodeWrapper(inspect.getsource(item.function), "python")
            results[f"{name}.source_code"] = source_code


def _get_shared_cwd_id(cls: type) -> Optional[str]:
    """
    Extract the id of the shared cwd directory needed by the class, if any.

    If the class is marked with shared_cwd multiple times only the last one
    is taken into account.

    Return the id or None.
    """
    if hasattr(cls, "pytestmark"):
        for mark in cls.pytestmark:
            if mark.name == "shared_cwd":
                return mark.args[0]
    return None


def _path_for_shared_cwd(config: pytest.Config, cwd_id: str) -> Path:
    """
    Return the path to the shared directory identified by id.
    """
    name = f"gaudi.{get_platform()}.{cwd_id}".replace("/", "_")
    return config.cache.mkdir(name)


@pytest.fixture(scope="class")
def fixture_result(
    request: pytest.FixtureRequest,
    tmp_path_factory: Callable,
) -> Generator[FixtureResult, None, None]:
    cls = request.cls
    result = None
    if cls and issubclass(cls, SubprocessBaseTest):
        if hasattr(cls, "popen_kwargs") and "cwd" not in cls.popen_kwargs:
            cwd_id = _get_shared_cwd_id(cls)
            cls.popen_kwargs["cwd"] = (
                _path_for_shared_cwd(request.config, cwd_id)
                if cwd_id
                else tmp_path_factory.mktemp("workdir")
            )
        result = cls.run_program(
            tmp_path=tmp_path_factory.mktemp("tmp-", numbered=True)
        )

    yield result


@pytest.fixture(scope="class")
def completed_process(
    fixture_result: FixtureResult,
) -> Generator[subprocess.CompletedProcess, None, None]:
    yield fixture_result.completed_process if fixture_result else None


@pytest.fixture(scope="class")
def stdout(
    completed_process: subprocess.CompletedProcess,
) -> Generator[bytes, None, None]:
    yield completed_process.stdout if completed_process else None


@pytest.fixture(scope="class")
def stderr(
    completed_process: subprocess.CompletedProcess,
) -> Generator[bytes, None, None]:
    yield completed_process.stderr if completed_process else None


@pytest.fixture(scope="class")
def returncode(
    completed_process: subprocess.CompletedProcess,
) -> Generator[int, None, None]:
    yield completed_process.returncode if completed_process else None


@pytest.fixture(scope="class")
def cwd(fixture_result: FixtureResult) -> Generator[Path, None, None]:
    yield Path(fixture_result.cwd) if fixture_result else None


@pytest.fixture(autouse=True)
def check_for_exceptions(
    request: pytest.FixtureRequest, fixture_result: FixtureResult
) -> None:
    if (
        fixture_result
        and fixture_result.run_exception is not None
        and "test_fixture_setup" not in request.keywords
    ):
        pytest.skip(f"{fixture_result.run_exception}")


@pytest.fixture(scope="class", autouse=True)
def capture_class_docstring(
    request: pytest.FixtureRequest,
) -> None:
    cls = request.cls
    if cls and cls.__doc__:
        results[f"{cls.__name__}.doc"] = inspect.getdoc(cls)


@pytest.fixture(scope="class")
def reference_path(request) -> Generator[Optional[Path], None, None]:
    cls = request.cls

    if not hasattr(cls, "reference") or cls.reference is None:
        yield None

    elif isinstance(cls.reference, (str, os.PathLike)):
        path = cls.reference
        if hasattr(cls, "resolve_path"):
            path = cls.resolve_path(path)

        path = expand_reference_file_name(path)
        yield Path(path) if path else None

    elif isinstance(cls.reference, dict):
        yield None

    else:
        raise ValueError(
            f"Invalid reference type {type(cls.reference)}, only str, PathLike or dict are allowed"
        )


@pytest.fixture(scope="class")
def reference(request, reference_path: Optional[Path]) -> Generator[dict, None, None]:
    cls = request.cls
    original_reference_data = None
    current_reference_data = None

    if reference_path:
        if reference_path.exists() and reference_path.stat().st_size > 0:
            with open(reference_path, "r") as f:
                original_reference_data = yaml.safe_load(f)
        else:
            # if the file does not exist we may have a relative path, so
            # we have to resolve it wrt the file containing the test class
            reference_path = file_path_for_class(cls).parent / reference_path
            original_reference_data = AlwaysTrueDefaultDict(lambda: None)

        current_reference_data = original_reference_data.copy()

    elif hasattr(cls, "reference") and isinstance(cls.reference, dict):
        original_reference_data = cls.reference
        current_reference_data = original_reference_data.copy()

    yield current_reference_data

    if current_reference_data == original_reference_data:
        return

    if reference_path:
        cnt = 0
        newrefname = f"{reference_path}.new"
        while os.path.exists(newrefname):
            cnt += 1
            newrefname = f"{reference_path}.~{cnt}~.new"

        if os.environ.get("GAUDI_TEST_IGNORE_STDOUT_VALIDATION") == "1":
            # when we ignore stdout validation we just overwrite the reference file
            newrefname = reference_path

        os.makedirs(os.path.dirname(newrefname), exist_ok=True)
        with open(newrefname, "w") as f:
            yaml.safe_dump(current_reference_data, f, sort_keys=False)

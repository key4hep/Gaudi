###############################################################################
# (c) Copyright 2024 CERN for the benefit of the LHCb Collaboration           #
#                                                                             #
# This software is distributed under the terms of the GNU General Public      #
# Licence version 3 (GPL Version 3), copied verbatim in the file "COPYING".   #
#                                                                             #
# In applying this licence, CERN does not waive the privileges and immunities #
# granted to it by virtue of its status as an Intergovernmental Organization  #
# or submit itself to any jurisdiction.                                       #
###############################################################################
"""
pytest plugin that report collected pytest files as CTest tests

This plugin is not meant to be used directly, but it is invoked by the
CMake function `gaudi_add_pytest()`
"""

import argparse
import os
from collections import defaultdict
from pathlib import Path


def pytest_addoption(parser, pluginmanager):
    parser.addoption(
        "--ctest-output-file",
        type=argparse.FileType("w"),
        help="name of the file to write to communicate to ctest the discovered tests",
    )
    parser.addoption(
        "--ctest-pytest-command",
        default="pytest",
        help="how pytest has to be invoked (e.g. using wrapper commands)",
    )
    parser.addoption(
        "--ctest-pytest-root-dir",
        default=Path.cwd(),
        help="root directory to compute test names",
    )
    parser.addoption(
        "--ctest-prefix",
        default="",
        help="string to prefix to the generated test names",
    )
    parser.addoption(
        "--ctest-label",
        default=["pytest"],
        action="append",
        help="labels to attach to the test (the label pytest is always added)",
    )
    parser.addoption(
        "--ctest-properties",
        default=[],
        action="append",
        help="test properties to set for all discovered tests",
    )
    parser.addoption(
        "--ctest-binary-dir",
        default=None,
        help="value of CMAKE_CURRENT_BINARY_DIR from which gaudi_add_pytest was invoked",
    )
    parser.addoption(
        "--ctest-coverage",
        default="",
        help="select modules for which produce coverage reports",
    )
    parser.addoption(
        "--ctest-coverage-command",
        default="coverage report",
        help="how coverage should be invoked to produce the final report",
    )


def pytest_collectstart(collector):
    session, config = collector.session, collector.config
    args = {
        name[6:]: getattr(config.option, name)
        for name in dir(config.option)
        if name.startswith("ctest_")
    }

    if args.get("binary_dir"):
        # $CMAKE_CURRENT_BINARY_DIR will be set by CTest when the test is run
        # so, for consistency, we set it at collection time too
        os.environ["CMAKE_CURRENT_BINARY_DIR"] = args["binary_dir"]

    session.ctest_args = args


def pytest_collection_modifyitems(session, config, items):
    if not session.ctest_args.get("output_file"):
        # nothing to do if no output file is specified
        return

    session.ctest_files = set(item.path for item in items)
    session.ctest_fixture_setup = defaultdict(set)
    session.ctest_fixture_required = defaultdict(set)
    for item in items:
        for marker in ("ctest_fixture_setup", "ctest_fixture_required"):
            for mark in item.iter_markers(name=marker):
                getattr(session, marker)[item.path].update(mark.args)


TEST_DESC_TEMPLATE = """
add_test({name} {pytest_cmd} {path})
set_tests_properties({name} PROPERTIES {properties})
"""


def pytest_collection_finish(session):
    args = session.ctest_args
    output = args.get("output_file")
    if not output:
        # nothing to do if no output file is specified
        return

    coverage = args["coverage"].split(",") if args["coverage"] else []
    if coverage:
        args["pytest_command"] += " --cov-append --cov-report= " + " ".join(
            f"--cov={module}" for module in coverage
        )

    properties = 'LABELS "{}" '.format(";".join(args["label"]))
    if args.get("binary_dir"):
        properties += f'ENVIRONMENT "CMAKE_CURRENT_BINARY_DIR={args["binary_dir"]}" '
    properties += " ".join(args["properties"])

    names = []
    for path in sorted(session.ctest_files):
        name = (
            args["prefix"] + os.path.relpath(path, args["pytest_root_dir"])
        ).replace("/", ".")
        if name.endswith(".py"):
            name = name[:-3]
        pytest_cmd = args["pytest_command"]

        if coverage and not names:
            # do not use --cov-append for the first test so that it resets the
            # stats
            pytest_cmd = pytest_cmd.replace("--cov-append", "")

        output.write(
            TEST_DESC_TEMPLATE.format(
                name=name,
                path=path,
                pytest_cmd=pytest_cmd,
                properties=properties,
            )
        )

        if session.ctest_fixture_setup.get(path):
            output.write(
                'set_tests_properties('
                f'{name} PROPERTIES FIXTURES_SETUP "{";".join(session.ctest_fixture_setup[path])}")\n'
            )

        if session.ctest_fixture_required.get(path):
            output.write(
                'set_tests_properties('
                f'{name} PROPERTIES FIXTURES_REQUIRED "{";".join(session.ctest_fixture_required[path])}")\n'
            )

        # we force one test to be run one by one
        if coverage and names:
            if names:
                output.write(
                    f"set_tests_properties({name} PROPERTIES DEPENDS {names[-1]})\n"
                )
                output.write(
                    f"set_tests_properties({name} PROPERTIES FIXTURES_SETUP {name})\n"
                )
        names.append(name)

    if coverage and names:
        name = (args["prefix"] + "coverage_report").replace("/", ".")
        output.write(
            TEST_DESC_TEMPLATE.format(
                name=name,
                path="",
                pytest_cmd=f"{args['coverage_command']}",
                properties=properties + " LABELS coverage",
            )
        )
        # coverage reports require all related tests to be run first
        output.write(
            f"set_tests_properties({name} PROPERTIES FIXTURES_REQUIRED \"{';'.join(names)}\")\n"
        )

    output.close()


def pytest_configure(config):
    config.addinivalue_line(
        "markers",
        "ctest_fixture_setup(name): mark test to set up a fixture needed by another test",
    )
    config.addinivalue_line(
        "markers",
        "ctest_fixture_required(name): mark test to require a fixture set up by another test",
    )

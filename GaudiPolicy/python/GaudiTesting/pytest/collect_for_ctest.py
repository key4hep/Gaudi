###############################################################################
# (c) Copyright 2024-2025 CERN for the benefit of the LHCb Collaboration      #
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

import os
import re
from collections import defaultdict
from pathlib import Path


def pytest_addoption(parser, pluginmanager):
    parser.addoption(
        "--ctest-output-file",
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
    output_filename = args.get("output_file")

    if not output_filename:
        # nothing to do if no output file is specified
        return

    output = open(output_filename, "w")
    output_rootdir = Path(output_filename).parent

    coverage = args["coverage"].split(",") if args["coverage"] else []
    if coverage:
        args["pytest_command"] += " --cov-report= --cov-reset " + " ".join(
            f"--cov={module}" for module in coverage
        )

    properties = 'LABELS "{}" '.format(";".join(args["label"]))
    if args.get("binary_dir"):
        properties += f'ENVIRONMENT "CMAKE_CURRENT_BINARY_DIR={args["binary_dir"]}" '
    properties += " ".join(args["properties"])

    producers = defaultdict(list)  # test name -> list of fixtures it produces
    consumers = defaultdict(list)  # test name -> list of fixtures it depends on
    fixtures = defaultdict(list)  # fixture name -> list of tests that produce it
    names = []
    for path in sorted(session.ctest_files):
        name = os.path.relpath(path, args["pytest_root_dir"]).replace("/", ".")
        if name.endswith(".py"):
            name = name[:-3]
        if name == ".":
            # when we pass a single file to pytest, pytest_root_dir and path are the same
            # and relpath returns ".", so we take the prefix and drop the final dot
            # (see https://gitlab.cern.ch/gaudi/Gaudi/-/issues/354)
            name = args["prefix"][:-1]
        else:
            name = args["prefix"] + name
        pytest_cmd = args["pytest_command"]

        output.write(
            TEST_DESC_TEMPLATE.format(
                name=name,
                path=path,
                pytest_cmd=pytest_cmd,
                properties=properties,
            )
        )

        if session.ctest_fixture_setup.get(path):
            for fixture in session.ctest_fixture_setup[path]:
                producers[name].append(fixture)
                fixtures[fixture].append(name)

        if session.ctest_fixture_required.get(path):
            for fixture in session.ctest_fixture_required[path]:
                consumers[name].append(fixture)

        names.append(name)

    for name in names:
        if name in producers:
            output.write(
                'set_tests_properties('
                f'{name} PROPERTIES FIXTURES_SETUP "{";".join(producers[name])}")\n'
            )

        if name in consumers:
            producer_test_names = []
            for fixture in consumers[name]:
                producer_test_names.extend(fixtures[fixture])
            output.write(
                'if (DEFINED ENV{PYTEST_DISABLE_FIXTURES_REQUIRED})\n'
                f'  set_tests_properties({name} PROPERTIES DEPENDS "{";".join(producer_test_names)}")\n'
                'else()\n'
                f'  set_tests_properties({name} PROPERTIES FIXTURES_REQUIRED "{";".join(consumers[name])}")\n'
                'endif()\n'
            )

        # we force one test to be run one by one
        if coverage:
            # generate one coverage file per test
            output.write(
                f"set_tests_properties({name} PROPERTIES ENVIRONMENT COVERAGE_FILE={output_rootdir}/.coverage.{name})\n"
                f"set_tests_properties({name} PROPERTIES FIXTURES_SETUP {name})\n"
            )

    if coverage and names:
        combine_test = (args["prefix"] + "coverage_combine").replace("/", ".")
        combine_command = re.sub(
            r" report .*",
            f" combine {' '.join(f'{output_rootdir}/.coverage.{n}' for n in names)}",
            args["coverage_command"],
        )
        output.write(
            TEST_DESC_TEMPLATE.format(
                name=combine_test,
                path="",
                pytest_cmd=combine_command,
                properties=properties,
            )
        )
        output.write(
            f"set_tests_properties({combine_test} PROPERTIES ENVIRONMENT COVERAGE_FILE={output_rootdir}/.coverage)\n"
            f"set_tests_properties({combine_test} PROPERTIES FIXTURES_REQUIRED \"{';'.join(names)}\")\n"
            f"set_tests_properties({combine_test} PROPERTIES FIXTURES_SETUP {combine_test})\n"
        )

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
            f"set_tests_properties({name} PROPERTIES ENVIRONMENT COVERAGE_FILE={output_rootdir}/.coverage)\n"
            f"set_tests_properties({name} PROPERTIES FIXTURES_REQUIRED {combine_test})\n"
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

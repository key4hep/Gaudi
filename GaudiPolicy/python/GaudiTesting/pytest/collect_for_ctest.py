###############################################################################
# (c) Copyright 2023 CERN for the benefit of the LHCb Collaboration           #
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


def pytest_collection_modifyitems(session, config, items):
    args = {
        name[6:]: getattr(config.option, name)
        for name in dir(config.option)
        if name.startswith("ctest_")
    }
    session.ctest_args = args
    session.ctest_files = set(
        item.path if hasattr(item, "path") else Path(item.fspath) for item in items
    )


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

    properties = 'LABELS "{}" '.format(";".join(args["label"]))
    properties += " ".join(args["properties"])

    for path in sorted(session.ctest_files):
        name = (
            args["prefix"] + os.path.relpath(path, args["pytest_root_dir"])
        ).replace("/", ".")
        if name.endswith(".py"):
            name = name[:-3]
        output.write(
            TEST_DESC_TEMPLATE.format(
                name=name,
                path=path,
                pytest_cmd=args["pytest_command"],
                properties=properties,
            )
        )

    output.close()

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
from ast import literal_eval
from pathlib import Path
from subprocess import run

import pytest


def config():
    # only the _merge_confDb.py in InstallArea/python/* are used automatically
    import Test_confDb  # noqa: F401

    # ------------------------------------------------------------------------------
    # https://its.cern.ch/jira/browse/GAUDI-253
    from Configurables import MyTestTool

    MyTestTool().PubToolHndl = "Public1"
    MyTestTool().PrivToolHndl = "Pivate1"
    MyTestTool().PrivEmptyToolHndl = ""

    MyTestTool().Int = 42
    MyTestTool().Text = "value"
    MyTestTool().DataHandle = "/Event/X"
    # ------------------------------------------------------------------------------


@pytest.fixture
def pickle_file(monkeypatch, tmp_path):
    monkeypatch.setenv(
        "PYTHONPATH",
        f"{Path(__file__).parent.parent / 'python'}:{os.environ.get('PYTHONPATH', '')}",
    )
    pickle_name = str(tmp_path / "options.pkl")
    run(
        [
            "gaudirun.py",
            "--verbose",
            "--dry-run",
            "--output",
            pickle_name,
            f"{__file__}:config",
        ],
        check=True,
    )
    yield pickle_name


def test(pickle_file: str, tmp_path):
    dump_name = str(tmp_path / "dump.py")
    run(
        ["gaudirun.py", "--verbose", "--dry-run", "--output", dump_name, pickle_file],
        check=True,
    )
    with open(dump_name) as dump:
        options_dump = literal_eval(dump.read())

    expected = {
        "ToolSvc.MyTestTool": {
            "PubToolHndl": "Public1",
            "PrivToolHndl": "Pivate1",
            "PrivEmptyToolHndl": "",
            "Int": 42,
            "Text": "value",
            "DataHandle": "/Event/X",
        }
    }

    assert options_dump == expected

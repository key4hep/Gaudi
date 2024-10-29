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
from ast import literal_eval
from pathlib import Path
from subprocess import run

import pytest


@pytest.fixture
def pickle_file(tmp_path, monkeypatch):
    monkeypatch.delenv("GAUDIAPPNAME", raising=False)
    monkeypatch.delenv("GAUDIAPPVERSION", raising=False)
    pickle_name = str(tmp_path / "options.pkl")
    run(
        [
            "gaudirun.py",
            "--verbose",
            "--dry-run",
            "--output",
            pickle_name,
            str(Path(__file__).parent.parent.parent / "pyjobopts" / "GAUDI-215.opts"),
        ],
        check=True,
    )
    yield pickle_name


def test(pickle_file: str, tmp_path):
    """
    https://its.cern.ch/jira/browse/GAUDI-215
    """

    dump_name = str(tmp_path / "dump.py")
    run(
        ["gaudirun.py", "--verbose", "--dry-run", "--output", dump_name, pickle_file],
        check=True,
    )
    with open(dump_name) as dump:
        options_dump = literal_eval(dump.read())

    expected = {"MyAlg": {"MyOpt": 1}, "OtherAlg": {"OtherOpt": 1}}

    assert options_dump == expected

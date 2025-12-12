#####################################################################################
# (c) Copyright 2021-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
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
from tempfile import NamedTemporaryFile

import pytest
from GaudiTests import run_gaudi


def gen_opts_dict(options):
    with NamedTemporaryFile(mode="w+", suffix=".py") as tmp:
        p = run_gaudi("--dry-run", "--output", tmp.name, options, capture_output=True)
        # make sure there are no warnings while loading generated options
        assert p.stderr == b""
        return literal_eval(tmp.read())


options = [
    Path(__file__).parent.parent.parent
    / "options"
    / "FunctionalAlgorithms"
    / "ProduceConsume.py",
]


@pytest.fixture(scope="module", params=options)
def options_expected(request):
    yield request.param, gen_opts_dict(request.param)


@pytest.mark.parametrize("filetype", ["opts", "pkl", "json", "yaml", "yml"])
def test_opts_dump(filetype, options_expected):
    opts, expected = options_expected
    with NamedTemporaryFile(suffix=f".{filetype}") as dump:
        run_gaudi("--dry-run", "--output", dump.name, opts)
        configuration = gen_opts_dict(dump.name)
        assert configuration == expected

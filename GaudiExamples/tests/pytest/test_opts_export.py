#####################################################################################
# (c) Copyright 2021 CERN for the benefit of the LHCb and ATLAS collaborations      #
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
        run_gaudi("--dry-run", "--output", tmp.name, options)
        return literal_eval(tmp.read())


@pytest.mark.parametrize(
    "options",
    [
        Path(__file__).parent.parent.parent
        / "options"
        / "FunctionalAlgorithms"
        / "ProduceConsume.py",
    ],
)
@pytest.mark.parametrize("filetype", ["opts", "pkl", "json", "yaml", "yml"])
def test_opts_dump(options, filetype):
    expected = gen_opts_dict(options)
    with NamedTemporaryFile(suffix=f".{filetype}") as dump:
        run_gaudi("--dry-run", "--output", dump.name, options)
        configuration = gen_opts_dict(dump.name)
        assert configuration == expected

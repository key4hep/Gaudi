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
from typing import Generator, List

import pytest
from GaudiTesting import GaudiExeTest


class OptParseTest(GaudiExeTest):
    """
    Helper class to write tests on options parsing.

    Just define the class member `options` (as for GaudiExeTest) and `expected_dump`
    (as a Python dictionary in the format produced buy the option `--output` of `gaudirun.py`).
    """

    command = ["gaudirun.py", "--dry-run", "--verbose"]

    expected_dump = None

    @classmethod
    def _prepare_command(cls, tmp_path=Path()) -> List[str]:
        command = super()._prepare_command(tmp_path=tmp_path)
        command.extend(
            ["--output", os.path.join(cls.popen_kwargs.get("cwd"), "opts.dump.py")]
        )
        return command

    def test_options(self, options_dump):
        assert (
            self.expected_dump is not None
        ), "expected_dump must be defined for OptParseTest"
        assert options_dump == self.expected_dump


@pytest.fixture(scope="class")
def options_dump(
    request: pytest.FixtureRequest,
    cwd: Path,
) -> Generator[dict, None, None]:
    cls = request.cls
    result = None
    if cls and issubclass(cls, OptParseTest):
        with open(cwd / "opts.dump.py") as dump:
            result = literal_eval(dump.read())

    yield result

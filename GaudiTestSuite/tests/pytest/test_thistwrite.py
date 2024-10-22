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
import pytest
from GaudiTesting import GaudiExeTest
from GaudiTesting.preprocessors import (
    RegexpReplacer,
    SortGroupOfLines,
    normalizeTestSuite,
)


@pytest.mark.ctest_fixture_setup("gauditestsuite.thistwrite")
@pytest.mark.shared_cwd("thist")
class Test(GaudiExeTest):
    command = ["Gaudi.exe", "../../options/THistWrite.opts"]
    reference = "refs/THistWrite.yaml"

    preprocessor = (
        normalizeTestSuite
        + RegexpReplacer(
            when="^THistSvc",
            orig=r"(stream: (rec|upd)  name: tuple[0-9].rt  size: )([0-9]+)",
            repl=r"\1###",
        )
        + RegexpReplacer(
            when="^THistSvc", orig=r"(.+M: )([0-9]+)(.*)", repl=r"\1########\3"
        )
        + RegexpReplacer(
            when="- id: ", orig=r"(.+M: )([0-9]+)(.*)", repl=r"\1########\3"
        )
        + RegexpReplacer(
            when="^ - 0x########.*id: ",
            orig=r"(.+id: )(\/.*\/.*)( t: )([0,1])( s: )([0,1])( M: )([0-9]+)(.*o: 0x######## )(.*)",
            repl=r"\1\2\3\4\5\6\7########\9\10",
        )
        + SortGroupOfLines(r"^ -")
        + SortGroupOfLines(r"^THistSvc\s+DEBUG finalize")
    )

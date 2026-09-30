#####################################################################################
# (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import re
import time

import pytest
from GaudiTesting.preprocessors import LineSkipper

LINES = [
    "Alg                 WARNING Cannot write to /tmp",
    "Alg                    INFO Cannot write to /tmp",
    "WARNING at start",
    "no match here",
    "",
    " | " + " " * 200 + "|",
]


@pytest.mark.parametrize(
    "regexp",
    [
        r".* +WARNING Cannot write to .*",
        r".*WARNING",
        r".*?WARNING",
        r".*+WARNING",
        r".*",
        r".*|nothing",
        r"^.*WARNING",
        re.compile(r".*WARNING"),
    ],
)
def test_line_skipper_matches_unchanged(regexp):
    pattern = re.compile(regexp)
    expected = [l for l in LINES if l and not pattern.search(l)]
    assert LineSkipper(regexps=[regexp])(LINES) == expected


def test_line_skipper_leading_wildcard_is_fast():
    lines = [" | " + " " * 400 + "| x"] * 200
    start = time.perf_counter()
    LineSkipper(regexps=[r".* +WARNING Cannot write to .*"])(lines)
    assert time.perf_counter() - start < 1

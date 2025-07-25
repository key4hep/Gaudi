#####################################################################################
# (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import pytest


def validate(stdout):
    expected = {
        "MapIntDouble": {1: 0.1, 2: 0.2, 3: 0.3},
        "MapIntInt": {1: 10, 2: 20, 3: 30},
        "MapIntString": {-1: "minus one", 0: "zero", 2: "two"},
        "MapStringDouble": {"aa": 0.1, "bb": 0.2, "cc": 3.0},
        "MapStringInt": {"a": 1, "b": 2, "c": 3},
        "MapStringString": {
            "a": "sddsgsgsdgdggf",
            "b": "sddsgsgsdgdggf",
            "c": "sddsgsgsdgdggf",
        },
        "MapStringVectorOfDoubles": {
            "aaa": [1.0, 2.0, 3.0],
            "bbb": [1.0, 2.0, 3.0],
            "ccc": [0.1, 0.2, 0.3],
        },
        "MapStringVectorOfInts": {"aaa": [1, 2, 3], "bbb": [4, 5, 6], "ccc": [7, 8, 9]},
        "MapStringVectorOfStrings": {
            "aaa": ["a", "b", "c"],
            "bbb": ["a", "b", "c"],
            "ccc": ["a", "b", "c"],
        },
        "MapUIntString": {0: "UZero", 1: "UOne", 2: "UTwo"},
        "PairDD": (3141.592, 2.1828183),
        "PairII": (3, 2),
        "VectorOfPairsDD": [
            (0.0, 1.0),
            (1.0, 2.0),
            (2.0, 3.0),
            (3.0, 4.0),
            (4.0, 5.0),
            (5.0, 6.0),
            (6.0, 7.0),
            (7.0, 8.0),
        ],
        "VectorOfPairsII": [(1, 1), (2, 1), (3, 2), (4, 3), (5, 5)],
        "VectorOfVectorsDouble": [[0.0, 1.0, 2.0], [0.0, -0.5, -0.25]],
        "VectorOfVectorsString": [["a", "b", "c"], ["A", "B", "C"]],
        "EmptyMap": {},
        "EmptyVector": [],
        "TupleStringIntDouble": ("hello", 10, 0.001),
        "TupleString": ("hello",),
        "SetOfInt": {1, 2},
        "SetOfString": {"a", "b"},
        "StdArrayDouble3": (3.3, 2.2, 1.1),
        "StdArrayInt1": (42,),
        "GaudiMapSS": {"a": "1", "b": "2"},
        "PTupleVector": [("one", "two", "three"), ("a", "b", "c")],
        "PIntVectorTuple": (42, ["one", "two", "three"]),
        "PTupleSet": [("a", "b", "c"), ("one", "two", "three")],
        "PIntSetTuple": (42, ["one", "three", "two"]),
        "PTupleUnSet": {("a", "b", "c"), ("one", "two", "three")},
        "PIntUnSetTuple": (42, {"one", "two", "three"}),
    }

    import re

    signature = re.compile(r"xProps\s*SUCCESS\s*My Properties :")
    prop_sign = re.compile(r"""xProps\s*SUCCESS\s*'([^']*)'\s*:(.*)""")

    output = map(lambda l: l.rstrip(), stdout.splitlines())
    i = output.__iter__()
    try:
        while not signature.match(next(i)):
            pass

        properties = {}
        for l in i:
            m = prop_sign.match(l)
            if m:
                properties[m.group(1)] = eval(m.group(2))
            else:
                break

        assert properties == expected

    except StopIteration:
        pytest.fail("Missing signature")

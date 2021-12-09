#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
def validate(stdout, stderr, result, causes):
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
        "StdArrayDouble3": (3.3, 2.2, 1.1),
        "StdArrayInt1": (42,),
        "GaudiMapSS": {"a": "1", "b": "2"},
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

        if properties != expected:
            causes.append("properties")
            from pprint import PrettyPrinter

            pp = PrettyPrinter()
            result["GaudiTest.properties.expected"] = result.Quote(pp.pformat(expected))
            result["GaudiTest.properties.found"] = result.Quote(pp.pformat(properties))
            pkeys = set(properties)
            ekeys = set(expected)
            missing = sorted(ekeys - pkeys)
            if missing:
                result["GaudiTest.properties.missing_keys"] = result.Quote(
                    pp.pformat(missing)
                )
            extra = sorted(pkeys - ekeys)
            if extra:
                result["GaudiTest.properties.extra_keys"] = result.Quote(
                    pp.pformat(extra)
                )
            mismatch = sorted(
                [k for k in ekeys.intersection(pkeys) if expected[k] != properties[k]]
            )
            if mismatch:
                result["GaudiTest.properties.mismatch_keys"] = result.Quote(
                    pp.pformat(mismatch)
                )

    except StopIteration:
        causes.append("missing signature")

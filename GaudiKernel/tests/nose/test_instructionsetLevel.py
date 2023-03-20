#####################################################################################
# (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from platform import processor
from subprocess import PIPE, Popen

from nose import SkipTest


def test():
    if processor() != "x86_64":
        raise SkipTest(
            "platform {} not supported (instructionsetLevel only works on x86_64".format(
                processor()
            )
        )
    out = Popen(["instructionsetLevel", "all"], stdout=PIPE).communicate()[0]
    out = out.decode("utf-8")
    known_flags = set(l.strip() for l in out.splitlines())

    expected = set()
    for l in open("/proc/cpuinfo"):
        if l.startswith("flags"):
            flags = set(l.strip().split())
            if "pni" in flags:
                flags.add("sse3")
            expected = flags.intersection(known_flags)
            break

    out = Popen(["instructionsetLevel"], stdout=PIPE).communicate()[0]
    out = out.decode("utf-8")
    found = set(l.strip() for l in out.splitlines())
    assert expected == found, "expected: {0}, found: {1}".format(
        sorted(expected), sorted(found)
    )

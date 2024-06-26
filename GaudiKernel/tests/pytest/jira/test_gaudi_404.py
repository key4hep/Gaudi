#!/usr/bin/env python3
#####################################################################################
# (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import os
import re
from subprocess import PIPE, run

import pytest
from GaudiTesting import platform_matches


@pytest.mark.skipif(
    platform_matches(["win", "mac"]),
    reason="Unsupported platform",
)
def test():
    """
    Test that the there is no signature for MsgStream operator<< <char []> in
    GaudiKernel.
    See https://its.cern.ch/jira/browse/GAUDI-404
    """
    # find libGaudiKernel.so in the LD_LIBRARY_PATH
    libname = "libGaudiKernel.so"

    searchpath = [os.path.curdir, "lib"]
    # The day we can run the test on other platforms we can do this:
    # varname = {'darwin': 'DYLD_LIBRARY_PATH',
    #           'win32': 'PATH'}.get(sys.platform, 'LD_LIBRARY_PATH')
    varname = "LD_LIBRARY_PATH"
    searchpath.extend(os.environ.get(varname, "").split(os.pathsep))

    libs = [
        p for p in (os.path.join(n, libname) for n in searchpath) if os.path.exists(p)
    ]
    assert libs, f"Cannot find {libname!r} in {searchpath}"

    nm = run(["nm", "-C", libs[0]], stdout=PIPE, text=True)
    assert nm.returncode == 0, "nm invocation failed"

    signature = re.compile(r"MsgStream& operator<< <char \[\d+\]>")
    lines = [line for line in nm.stdout.splitlines() if signature.search(line)]

    assert not lines, "found MsgStream operator<< specialization"
    print("SUCCESS: no MsgStream operator<< specialization found")

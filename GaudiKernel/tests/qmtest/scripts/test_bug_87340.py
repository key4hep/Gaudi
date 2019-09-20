#!/usr/bin/env python
"""
Test that the there is no signature for MsgStream operator<< <char []> in
GaudiKernel.
See https://savannah.cern.ch/bugs?87340
"""
from __future__ import print_function
import os
import sys
import re

from subprocess import Popen, PIPE

# find libGaudiKernel.so in the LD_LIBRARY_PATH
libname = 'libGaudiKernel.so'

searchpath = [os.path.curdir, 'lib']
# The day we can run the test on other platforms we can do this:
# varname = {'darwin': 'DYLD_LIBRARY_PATH',
#           'win32': 'PATH'}.get(sys.platform, 'LD_LIBRARY_PATH')
varname = 'LD_LIBRARY_PATH'
searchpath.extend(os.environ.get(varname, "").split(os.pathsep))

try:
    lib = next(
        p for p in (os.path.join(n, libname) for n in searchpath)
        if os.path.exists(p))
except StopIteration:
    print(
        'FAILURE: Cannot find',
        repr(libname),
        'in',
        searchpath,
        file=sys.stderr)
    sys.exit(2)

nm = Popen(["nm", '-C', lib], stdout=PIPE)
output, _ = nm.communicate()
output = output.decode('utf-8')

if nm.returncode:
    print(output)
    print('FAILURE: nm call failed', file=sys.stderr)
    sys.exit(nm.returncode)

signature = re.compile(r"MsgStream&amp; operator&lt;&lt; &lt;char \[\d+\]&gt;")

lines = list(filter(signature.search, output.splitlines()))
if lines:
    print("\n".join(lines))
    print(
        "FAILURE: found MsgStream operator<< specialization", file=sys.stderr)
    sys.exit(1)
else:
    print("SUCCESS: no MsgStream operator<< specialization found")

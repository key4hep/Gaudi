#!/usr/bin/env python

import sys
from subprocess import Popen
from pprint import pprint

def run(cmd):
    print "--- Running", cmd
    sys.stdout.flush()
    retval = Popen(cmd).wait()
    if retval:
        raise RuntimeError("Command failed")

options1 = ["--option", "import Test_confDb; from Configurables import MyConfUser; MyConfUser(EvtMax = 10)"]
options2 = ["--option", "import Test_confDb; from Configurables import MyConfUser; MyConfUser(OutputLevel = 0)"]

pickle_file = "test.pkl"
print "-----", "Prepare pickle file from first options"
run(["gaudirun.py", "-n", "-o", pickle_file] + options1)

print "-----", "case 1"
run(["gaudirun.py", "-n", "-o", "case1.py" ] + options1 + options2)
case1 = eval(open("case1.py").read())
pprint(case1)

print "-----", "case 2"
run(["gaudirun.py", "-n", "-o", "case2.py", "test.pkl" ] + options2)
case2 = eval(open("case2.py").read())
pprint(case2)

if case1 != case2:
    raise RuntimeError("case 1 and case 2 differ")

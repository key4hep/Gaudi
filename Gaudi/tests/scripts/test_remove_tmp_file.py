#!/usr/bin/env python
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

import os
from subprocess import call

tmpfile = os.path.join(os.getcwd(), "tmp.opts")

with open(tmpfile, "w") as f:
    f.write("MyAlg.MyOpt = 1;\n")

os.environ["GAUDI_TEMP_OPTS_FILE"] = tmpfile
os.environ["GAUDIAPPNAME"] = ""
os.environ["GAUDIAPPVERSION"] = ""

assert call(["gaudirun.py", "--dry-run", "--output", "out.py"]) == 0

assert not os.path.exists(tmpfile)

assert os.path.exists("out.py")

expected = {"MyAlg": {"MyOpt": 1}}
observed = eval(open("out.py").read())

assert expected == observed

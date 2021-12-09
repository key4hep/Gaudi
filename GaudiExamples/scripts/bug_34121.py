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
# ====================================================================
from Configurables import bug_34121__MyAlgorithm as MyAlgorithm
from Configurables import bug_34121__Tool as Tool
from Gaudi.Configuration import *

tool = Tool(Double=-1)

alg = MyAlgorithm("Alg")
alg.addTool(tool, name="Tool")

ApplicationMgr(EvtSel="NONE", TopAlg=[alg])

# ====================================================================

import GaudiPython

app = GaudiPython.AppMgr()

t = app.tool(alg.Tool.getName())
for i in range(5):
    t.Double = i
    app.run(1)

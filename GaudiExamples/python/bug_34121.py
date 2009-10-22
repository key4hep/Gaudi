#!/usr/bin/env python
# ====================================================================
from Gaudi.Configuration import *
from Configurables import bug_34121__Tool as Tool, \
                          bug_34121__MyAlgorithm as MyAlgorithm

tool = Tool(Double = -1)

alg = MyAlgorithm("Alg")
alg.addTool(tool,name="Tool")

ApplicationMgr(EvtSel = "NONE", TopAlg = [alg])

# ====================================================================

import GaudiPython

app = GaudiPython.AppMgr()

t = app.tool(alg.Tool.getName())
for i in range(5):
    t.Double = i
    app.run(1)

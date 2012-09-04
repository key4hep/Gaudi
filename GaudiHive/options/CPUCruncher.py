#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
# ============================================================================
from Configurables import GaudiExamplesCommonConf, CPUCruncher
#GaudiExamplesCommonConf()
# ============================================================================
# Setup the test
cpuc0= CPUCruncher ("cruncher_0", avgRuntime=.1)
cpuc1= CPUCruncher ("cruncher_1",varRuntime=.1, avgRuntime=.5)

# ============================================================================

app = ApplicationMgr()
app.TopAlg = [ cpuc0, cpuc1 ]
app.EvtSel = "NONE" # do not use any event input
app.EvtMax = 10

# ============================================================================
# The END
# ============================================================================

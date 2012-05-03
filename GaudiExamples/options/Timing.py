#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
# ============================================================================
from Configurables import GaudiExamplesCommonConf
GaudiExamplesCommonConf()
# ============================================================================
# @file
#  Simple file to run GaudiExamples::TimingAlg example
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @author Marco CLEMENCIC Marco.Clemencic@cern.ch
#  @date 2008-05-13
# ============================================================================
app = ApplicationMgr()
#app.ExtSvc += [ "RndmGenSvc" ]
app.TopAlg = [ "GaudiExamples::TimingAlg/Timing" ]
app.EvtSel = "NONE" # do not use any event input
app.EvtMax = 400
# ============================================================================
# The END
# ============================================================================

#!/usr/bin/env gaudirun.py
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
app.TopAlg = ["GaudiExamples::TimingAlg/Timing"]
app.EvtSel = "NONE"  # do not use any event input
app.EvtMax = 400
# ============================================================================
# The END
# ============================================================================

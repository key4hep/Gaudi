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
from __future__ import print_function
###############################################################
# Job options file
###############################################################

from Gaudi.Configuration import *
from Configurables import ParentAlg, StopperAlg, TimingAuditor, HelloWorld
from Configurables import EventLoopMgr

from GaudiConfig.ControlFlow import seq

from Configurables import GaudiExamplesCommonConf
# see implementation of Gaudi_Test_MySuperAlg in GaudiExamples/Configuration.py
from Configurables import Gaudi_Test_MySuperAlg as MySuperAlg

GaudiExamplesCommonConf()

s1 = MySuperAlg('s1', OutputLevel=INFO)
s2 = MySuperAlg('s2', OutputLevel=WARNING)
top = s1 >> s2

MySuperAlg('s2', PercentPass=75, OutputLevel=DEBUG)

# -----------------------------------------------------------------
# Testing the new GaudiSequencer
# -----------------------------------------------------------------
sand = HelloWorld('AND') & EventCounter('ANDCounter')
sor = HelloWorld('OR') | EventCounter('ORCounter')

try:
    MySuperAlg('AND')
    raise RuntimeError('we should not be able to retype to SuperAlgorithm')
except AssertionError:
    pass  # it's expected

all = ParentAlg() >> StopperAlg(StopCount=20) >> top >> sand >> sor

print('# --- Configured Control Flow Expression:')
print('#', all)
print('# ---')
EventLoopMgr(PrintControlFlowExpression=True)

# -----------------------------------------------------------------
ApplicationMgr(
    TopAlg=[all],
    EvtMax=10,  # events to be processed (default is 10)
    EvtSel='NONE',  # do not use any event input
    ExtSvc=['ToolSvc', 'AuditorSvc'],
    AuditAlgorithms=True)

AuditorSvc().Auditors.append(TimingAuditor("TIMER"))

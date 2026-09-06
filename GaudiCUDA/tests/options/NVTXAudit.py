#####################################################################################
# (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################

from Configurables import NVTXAuditor
from Configurables import CPUCruncher
from Configurables import Gaudi__Sequencer as Sequencer
from Configurables import AuditorSvc
from Gaudi.Configuration import *

# Application setup
app = ApplicationMgr()

# Auditors
AuditorSvc(Auditors=[NVTXAuditor(OutputLevel=INFO)])
app.AuditAlgorithms = True

# - Algorithms

InnerAlg1 = CPUCruncher("InnerAlg1", avgRuntime=0.01, OutputLevel=ERROR)
InnerAlg2 = CPUCruncher("InnerAlg2", avgRuntime=0.01, OutputLevel=ERROR)
OuterAlg = CPUCruncher("OuterAlg", avgRuntime=0.01, OutputLevel=ERROR)
sequencer = Sequencer(
    "Sequencer",
    Sequential=True,
    Members=[InnerAlg1, InnerAlg2],
    OutputLevel=ERROR,
)

app.TopAlg = [sequencer, OuterAlg]

# - Events
app.EvtMax = 4
app.EvtSel = "NONE"
app.HistogramPersistency = "NONE"

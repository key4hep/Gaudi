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

from Configurables import CpuHungryAlg, IntelProfilerAuditor
from Gaudi.Configuration import *

MessageSvc().OutputLevel = INFO

alg1 = CpuHungryAlg("Alg1")
alg2 = CpuHungryAlg("Alg2")
alg3 = CpuHungryAlg("Alg3")
alg4 = CpuHungryAlg("Alg4")

alg1.Loops = alg2.Loops = alg3.Loops = alg4.Loops = 5000000

subtop = Sequencer("SubSequence", Members=[alg1, alg2, alg3], StopOverride=True)
top = Sequencer("TopSequence", Members=[subtop, alg4], StopOverride=True)

profiler = IntelProfilerAuditor()
profiler.OutputLevel = DEBUG
profiler.StartFromEventN = 1
profiler.StopAtEventN = 2
profiler.ComponentsForTaskTypes = []
profiler.IncludeAlgorithms = ["SubSequence"]
profiler.ExcludeAlgorithms = ["Alg2"]
AuditorSvc().Auditors += [profiler]

ApplicationMgr(
    EvtMax=3,
    EvtSel="NONE",
    HistogramPersistency="NONE",
    TopAlg=[top],
    AuditAlgorithms=True,
)

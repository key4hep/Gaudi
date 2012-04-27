#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import IntelProfilerAuditor, CpuHungryAlg

MessageSvc().OutputLevel = INFO

alg1 = CpuHungryAlg("Alg1")
alg2 = CpuHungryAlg("Alg2")
alg3 = CpuHungryAlg("Alg3")
alg4 = CpuHungryAlg("Alg4")

alg1.Loops = alg2.Loops = alg3.Loops = alg4.Loops = 5000000

subtop = Sequencer('SubSequence', Members = [alg1, alg2, alg3], StopOverride = True )
top = Sequencer('TopSequence', Members = [subtop, alg4], StopOverride = True )

profiler = IntelProfilerAuditor()
profiler.OutputLevel = DEBUG
profiler.StartFromEventN = 1
profiler.StopAtEventN = 2
profiler.ComponentsForTaskTypes = []
profiler.IncludeAlgorithms = ["SubSequence"]
profiler.ExcludeAlgorithms = ["Alg2"]
AuditorSvc().Auditors +=  [profiler]

ApplicationMgr( EvtMax = 3,
                EvtSel = 'NONE',
                HistogramPersistency = 'NONE',
                TopAlg = [top],
                AuditAlgorithms=True)

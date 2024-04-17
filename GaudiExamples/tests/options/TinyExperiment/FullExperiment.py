#####################################################################################
# (c) Copyright 1998-2020 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from Configurables import ApplicationMgr, CheckerAlg, DigitizationAlg
from Configurables import Gaudi__Monitoring__MessageSvcSink as MessageSvcSink
from Configurables import GeneratorAlg, RandomGenSvc, SimulationAlg, TrackingAlg

# - Algorithms
RandomGenSvc(RandomSeed=1234)
gen = GeneratorAlg(NbTracksToGenerate=10)
sim = SimulationAlg(NbHitsPerTrack=15, MCTracksLocation=gen.MCTracksLocation)
digi = DigitizationAlg(SigmaNoise=0.1, MCHitsLocation=sim.MCHitsLocation)
track = TrackingAlg(NumberBins=100, Sensibility=6, HitsLocation=digi.HitsLocation)
check = CheckerAlg(DeltaThetaMax=0.01, TracksLocation=track.TracksLocation)

# this printout is useful to check that the type information is passed to python correctly
for configurable in [gen, sim, digi, track, check]:
    print(configurable)

# Application setup
app = ApplicationMgr(
    ExtSvc=[MessageSvcSink()],
    TopAlg=[gen, sim, digi, track, check],
    EvtMax=100,
    EvtSel="NONE",
)

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
import sys


def config(evtslots=12, threads=10):
    from Configurables import (
        ApplicationMgr,
        AvalancheSchedulerSvc,
        CheckerAlg,
        DigitizationAlg,
    )
    from Configurables import Gaudi__Monitoring__MessageSvcSink as MessageSvcSink
    from Configurables import (
        GeneratorAlg,
        HiveSlimEventLoopMgr,
        HiveWhiteBoard,
        RandomGenSvc,
        SimulationAlg,
        TrackingAlg,
    )

    evtslots = 12
    threads = 10

    # infrastructure and services
    RandomGenSvc(RandomSeed=1234)
    whiteboard = HiveWhiteBoard("EventDataSvc", EventSlots=evtslots)
    scheduler = AvalancheSchedulerSvc(ThreadPoolSize=threads)
    slimeventloopmgr = HiveSlimEventLoopMgr(SchedulerName=scheduler.name())

    # - Algorithms
    gen = GeneratorAlg(NbTracksToGenerate=10)
    sim = SimulationAlg(NbHitsPerTrack=15, MCTracksLocation=gen.MCTracksLocation)
    digi = DigitizationAlg(SigmaNoise=0.1, MCHitsLocation=sim.MCHitsLocation)
    track = TrackingAlg(NumberBins=100, Sensibility=6, HitsLocation=digi.HitsLocation)
    check = CheckerAlg(DeltaThetaMax=0.01, TracksLocation=track.TracksLocation)

    # this printout is useful to check that the type information is passed to python correctly
    for configurable in [gen, sim, digi, track, check]:
        print(configurable)
    # this is to make sure the output is actually printed
    sys.stdout.flush()

    # Application setup
    ApplicationMgr(
        ExtSvc=[MessageSvcSink(), whiteboard],
        TopAlg=[gen, sim, digi, track, check],
        EvtMax=10000,
        EvtSel="NONE",
        EventLoop=slimeventloopmgr,
    )

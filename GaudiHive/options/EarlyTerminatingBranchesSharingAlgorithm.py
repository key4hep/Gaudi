#!/usr/bin/env gaudirun.py
"""Test a CF configuration with an algorithm shared between branches that can terminate early."""

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveSlimEventLoopMgr, AvalancheSchedulerSvc, AlgResourcePool
from Configurables import GaudiSequencer, CPUCruncher

# metaconfig
evtslots = 1
evtMax = 1
algosInFlight = 1

whiteboard = HiveWhiteBoard(
    "EventDataSvc", EventSlots=evtslots, OutputLevel=INFO)

slimeventloopmgr = HiveSlimEventLoopMgr(
    SchedulerName="AvalancheSchedulerSvc", OutputLevel=DEBUG)

AvalancheSchedulerSvc(ThreadPoolSize=algosInFlight, OutputLevel=DEBUG)


def parOR(name, subs=[]):
    """ parallel OR sequencer """
    seq = GaudiSequencer(name)
    seq.ModeOR = True
    seq.Sequential = False
    seq.ShortCircuit = False
    #    seq.StopOverride = False
    for s in subs:
        seq.Members.append(s)
    return seq


def seqAND(name, subs=[]):
    """ sequential AND sequencer """
    seq = GaudiSequencer(name)
    seq.ModeOR = False
    seq.Sequential = True
    #    seq.StopOverride = True
    seq.ShortCircuit = True
    for s in subs:
        seq.Members.append(s)
    return seq


topSequence = GaudiSequencer("topSequence")

and1A = seqAND("AND1A")

filterA = CPUCruncher("filterA")
and1A.Members.append(filterA)

and2A = seqAND("AND2A")
#and2A.StopOverride = True
and1A.Members.append(and2A)

orA = parOR("ORA")
and2A.Members.append(orA)

and3A = seqAND("AND3A")
#and3A.StopOverride = True
orA.Members.append(and3A)

alg1 = CPUCruncher("Alg1")
and3A.Members.append(alg1)

hypoA = CPUCruncher("hypoA")
and2A.Members.append(hypoA)

#######
and1B = seqAND("AND1B")

filterB = CPUCruncher("filterB", InvertDecision=True)
and1B.Members.append(filterB)

and2B = seqAND("AND2B")
#and2B.StopOverride = True
and1B.Members.append(and2B)

orB = parOR("ORB")
and2B.Members.append(orB)

and3B = seqAND("AND3B")
#and3B.StopOverride = True
orB.Members.append(and3B)

alg2 = CPUCruncher("Alg2")
and3B.Members.append(alg1)

hypoB = CPUCruncher("hypoB")
and2B.Members.append(hypoB)

#######

topSequence.Members.append(and1A)
topSequence.Members.append(and1B)

ApplicationMgr(
    EvtMax=evtMax,
    EvtSel='NONE',
    ExtSvc=[whiteboard],
    EventLoop=slimeventloopmgr,
    TopAlg=[topSequence],
    MessageSvcType="InertMessageSvc",
    OutputLevel=INFO)

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

from Configurables import AlgTimingAuditor, ApplicationMgr, AuditorSvc, EventLoopMgr
from Configurables import Gaudi__Examples__EventCounter as EventCounter
from Configurables import Gaudi__Examples__Prescaler as Prescaler
from Configurables import GaudiExamplesCommonConf, HelloWorld, ParentAlg, StopperAlg
from Gaudi.Configuration import DEBUG
from GaudiConfig.ControlFlow import seq

###############################################################
# Job options file
# ==============================================================


GaudiExamplesCommonConf()

# --------------------------------------------------------------
# Testing Sequencers
# --------------------------------------------------------------
p1 = Prescaler("Prescaler1", PercentPass=50.0)
p2 = Prescaler("Prescaler2", PercentPass=10.0)
h = HelloWorld(OutputLevel=DEBUG)
c1 = EventCounter("Counter1")
c2 = EventCounter("Counter2")

s1 = seq(p1 & h & c1)
s2 = seq(p2 & h & c2)
top = s1 >> s2
# s1  = Sequencer('Sequence1', Members = [p1, h, c1] )
# s2  = Sequencer('Sequence2', Members = [p2, h, c2] )
# top = Sequencer('TopSequence', Members = [s1, s2], ShortCircuit = False )

# -----------------------------------------------------------------
# Testing the new GaudiSequencer
# -----------------------------------------------------------------
sand = HelloWorld("AND") & EventCounter("ANDCounter")
sor = HelloWorld("OR") | EventCounter("ORCounter")
# sand = GaudiSequencer( 'ANDSequence',
#                       Members = [ HelloWorld('AND'), EventCounter('ANDCounter') ],
#                       MeasureTime = 1 )
# sor =  GaudiSequencer( 'ORSequence',
#                       Members = [ HelloWorld('OR'), EventCounter('ORCounter') ],
#                       MeasureTime = 1,
#                       ModeOR = 1 )

all = ParentAlg() >> StopperAlg(StopCount=20) >> top >> sand >> sor

print("# --- Configured Control Flow Expression:")
print("#", all)
print("# ---")
EventLoopMgr(PrintControlFlowExpression=True)
# -----------------------------------------------------------------
ApplicationMgr(
    TopAlg=[all],
    EvtMax=10,  # events to be processed (default is 10)
    EvtSel="NONE",  # do not use any event input
    ExtSvc=["ToolSvc", "AuditorSvc", "Gaudi::Monitoring::MessageSvcSink"],
    AuditAlgorithms=True,
)

AuditorSvc().Auditors += [AlgTimingAuditor("TIMER")]

###############################################################
# Job options file
# ==============================================================

from Gaudi.Configuration import *
from Configurables import ParentAlg, StopperAlg, Prescaler, HelloWorld, TimingAuditor
from Configurables import EventLoopMgr

from GaudiConfig.ControlFlow import seq

from Configurables import GaudiExamplesCommonConf
GaudiExamplesCommonConf()

# --------------------------------------------------------------
# Testing Sequencers
# --------------------------------------------------------------
p1 = Prescaler('Prescaler1', PercentPass=50., OutputLevel=WARNING)
p2 = Prescaler('Prescaler2', PercentPass=10., OutputLevel=WARNING)
h = HelloWorld(OutputLevel=DEBUG)
c1 = EventCounter('Counter1')
c2 = EventCounter('Counter2')

s1 = seq(p1 & h & c1)
s2 = seq(p2 & h & c2)
top = s1 >> s2
#s1  = Sequencer('Sequence1', Members = [p1, h, c1] )
#s2  = Sequencer('Sequence2', Members = [p2, h, c2] )
#top = Sequencer('TopSequence', Members = [s1, s2], StopOverride = True )

# -----------------------------------------------------------------
# Testing the new GaudiSequencer
# -----------------------------------------------------------------
sand = HelloWorld('AND') & EventCounter('ANDCounter')
sor = HelloWorld('OR') | EventCounter('ORCounter')
# sand = GaudiSequencer( 'ANDSequence',
#                       Members = [ HelloWorld('AND'), EventCounter('ANDCounter') ],
#                       MeasureTime = 1 )
# sor =  GaudiSequencer( 'ORSequence',
#                       Members = [ HelloWorld('OR'), EventCounter('ORCounter') ],
#                       MeasureTime = 1,
#                       ModeOR = 1 )

all = ParentAlg() >> StopperAlg(StopCount=20) >> top >> sand >> sor

print '# --- Configured Control Flow Expression:'
print '#', all
print '# ---'
EventLoopMgr(PrintControlFlowExpression=True)
# -----------------------------------------------------------------
ApplicationMgr(
    TopAlg=[all],
    EvtMax=10,  # events to be processed (default is 10)
    EvtSel='NONE',  # do not use any event input
    ExtSvc=['ToolSvc', 'AuditorSvc'],
    AuditAlgorithms=True)

AuditorSvc().Auditors += [TimingAuditor("TIMER")]

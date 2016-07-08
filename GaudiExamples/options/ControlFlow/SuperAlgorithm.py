###############################################################
# Job options file
#==============================================================

from Gaudi.Configuration import *
from Configurables import ParentAlg, StopperAlg, Prescaler, HelloWorld, TimingAuditor
from Configurables import EventLoopMgr

from GaudiConfig.ControlFlow import seq
from GaudiKernel.Configurable import SuperAlgorithm

from Configurables import GaudiExamplesCommonConf
GaudiExamplesCommonConf()

#--------------------------------------------------------------
# Defining a SuperAlgorithm class
#--------------------------------------------------------------
class MySuperAlg(SuperAlgorithm):
    def __init__(self, name, **kwargs):
        p = Prescaler(name + '_Prescaler', PercentPass=50.)
        h = HelloWorld(name + '_HW')
        c = EventCounter(name + '_Counter')
        super(MySuperAlg, self).__init__(name, item=(p & h & c), **kwargs)

s1 = MySuperAlg('s1', OutputLevel=INFO)
s2 = MySuperAlg('s2', OutputLevel=WARNING)
top = s1 >> s2

s2.PercentPass = 75
s2.OutputLevel = DEBUG

#-----------------------------------------------------------------
# Testing the new GaudiSequencer
#-----------------------------------------------------------------
sand = HelloWorld('AND') & EventCounter('ANDCounter')
sor = HelloWorld('OR') | EventCounter('ORCounter')
#sand = GaudiSequencer( 'ANDSequence',
#                       Members = [ HelloWorld('AND'), EventCounter('ANDCounter') ],
#                       MeasureTime = 1 )
#sor =  GaudiSequencer( 'ORSequence',
#                       Members = [ HelloWorld('OR'), EventCounter('ORCounter') ],
#                       MeasureTime = 1,
#                       ModeOR = 1 )

all = ParentAlg() >> StopperAlg(StopCount=20) >> top >> sand >> sor

print '# --- Configured Control Flow Expression:'
print '#', all
print '# ---'
EventLoopMgr(PrintControlFlowExpression=True)
#-----------------------------------------------------------------
ApplicationMgr( TopAlg = [all],
                EvtMax = 10,     # events to be processed (default is 10)
                EvtSel = 'NONE', # do not use any event input
                ExtSvc = ['ToolSvc', 'AuditorSvc' ],
                AuditAlgorithms = True )

AuditorSvc().Auditors += [ TimingAuditor("TIMER") ]

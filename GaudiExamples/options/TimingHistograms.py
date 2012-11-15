###############################################################
# Job options file
#==============================================================

from Gaudi.Configuration import *
importOptions('AlgSequencer.py')

#--------------------------------------------------------------
# Enable Timing Histograms
#--------------------------------------------------------------
from Configurables import TimingAuditor, SequencerTimerTool
TIMER = TimingAuditor("TIMER")
TIMER.addTool(SequencerTimerTool, name="TIMER")
TIMER.TIMER.HistoProduce = True

#--------------------------------------------------------------
# Enable histograms output
#--------------------------------------------------------------
RootHistSvc().OutputFile = 'timing_histos.root'
ApplicationMgr(HistogramPersistency='ROOT')

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
###############################################################
# Job options file
# ==============================================================

from Gaudi.Configuration import *
importOptions('AlgSequencer.py')

# --------------------------------------------------------------
# Enable Timing Histograms
# --------------------------------------------------------------
from Configurables import TimingAuditor, SequencerTimerTool
TIMER = TimingAuditor("TIMER")
TIMER.addTool(SequencerTimerTool, name="TIMER")
TIMER.TIMER.HistoProduce = True

# --------------------------------------------------------------
# Enable histograms output
# --------------------------------------------------------------
RootHistSvc().OutputFile = 'timing_histos.root'
ApplicationMgr(HistogramPersistency='ROOT')

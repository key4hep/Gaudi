####################################################################
# Trick the IODataManager to expose bug #101513:
#   Raw->Raw processing should not print an error
# https://savannah.cern.ch/bugs/?101513
####################################################################

from Gaudi.Configuration import *
from Configurables import Gaudi__RootCnvSvc as RootCnvSvc, GaudiPersistency
from Configurables import ReadAlg, ReadTES, FileRecordDataSvc

import shutil
shutil.copy('bug_101513.dst', 'B0101513-0000-0000-0000-000000000000')

# I/O
GaudiPersistency()
FileCatalog(Catalogs = [ ])
esel = EventSelector(PrintFreq=50, FirstEvent=1)
esel.Input = ["DATAFILE='PFN:B0101513-0000-0000-0000-000000000000'  SVC='Gaudi::RootEvtSelector' OPT='READ'"]

# Application setup
app = ApplicationMgr()
# - Algorithms
app.TopAlg = [ ]
# - Events
app.EvtMax = 1
app.HistogramPersistency = "NONE"

#RootCnvSvc(OutputLevel=VERBOSE)
#ChronoStatSvc(OutputLevel=WARNING)
#SequencerTimerTool(OutputLevel=WARNING)
#MessageSvc(OutputLevel=VERBOSE)

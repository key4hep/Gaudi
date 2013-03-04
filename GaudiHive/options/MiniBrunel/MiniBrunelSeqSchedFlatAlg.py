from Gaudi.Configuration import *

from Configurables import AlgResourcePool,\
                          HiveSlimEventLoopMgr,\
                          HiveSlimEventLoopMgr,\
                          HiveWhiteBoard,\
                          HiveReadAlgorithm,\
                          SequentialSchedulerSvc,\
                          EventClockSvc,\
                          ODINDecodeTool
#-----------------------------------------------------------------
# WhiteBoard (replacing the one in Brunel)
whiteboard   = HiveWhiteBoard("EventDataSvc",
                              ForceLeaves        = True,
                              RootCLID           =    1,
                              EnableFaultHandler = True,
                              EventSlots = 1)
#-----------------------------------------------------------------
# Resource Pool
AlgResourcePool(OutputLevel = DEBUG,
                DoHacks=True)

#-----------------------------------------------------------------
# Event Loop Manager
slimeventloopmgr = HiveSlimEventLoopMgr(SchedulerName="SequentialSchedulerSvc",
                                        OutputLevel=INFO)
#-----------------------------------------------------------------
# Sequential Scheduler
SequentialSchedulerSvc(UseTopAlgList=False)

#-----------------------------------------------------------------
# Read Algorithm
GaudiPersistency()
loader = HiveReadAlgorithm("Loader",
                           OutputLevel=WARNING,
                           NeededResources = ['ROOTIO']
                           )
#-----------------------------------------------------------------
ApplicationMgr().ExtSvc = ['AlgResourcePool']
ApplicationMgr().ExtSvc += [ whiteboard ]
ApplicationMgr().EventLoop = slimeventloopmgr
ApplicationMgr().OutputLevel=INFO

IncidentSvc().OutputLevel=WARNING
EventClockSvc(OutputLevel=WARNING)
ODINDecodeTool(OutputLevel=WARNING)
#----------------------------------------------------------------------------
#Older options

#Conditions configuration
importOptions('$APPCONFIGOPTS/Brunel/DataType-2012.py')

# Input data
importOptions('$BRUNELROOT/options/COLLISION12-Beam4000GeV-VeloClosed-MagDown.py')

#### Application configuration ####
from Configurables import InertMessageSvc

# Enable InertMessageSvc
msgSvc = InertMessageSvc("MessageSvc")
ApplicationMgr(MessageSvcType=msgSvc.getType())
ApplicationMgr().SvcMapping.append(msgSvc)

# Fine tuning of Brunel
from Configurables import Brunel, RecSysConf, RecMoniConf, LumiAlgsConf
Brunel(
       OutputType='NONE', # Do not write output
       EvtMax=50,
      )
# Change input file
EventSelector().Input = [
  "DATAFILE='mdf:/afs/cern.ch/work/d/dpiparo/MiniBrunel/MiniBrunelInput.raw' SVC='LHCb::MDFSelector'"
    ]

# - limit reconstruction algorithms
RecSysConf(RecoSequence=['Decoding', 'VELO'])

# - do not run monitoring algorithms
RecMoniConf(MoniSequence=[])

from Configurables import LumiAlgsConf
def disableLumi():
    LumiAlgsConf().LumiSequencer.Members = []
appendPostConfigAction(disableLumi)


# Check the algos and outstreams
def printAppMan():
  print ApplicationMgr().OutStream
  print ApplicationMgr().TopAlg
appendPostConfigAction(printAppMan)

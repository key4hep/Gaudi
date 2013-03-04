from Gaudi.Configuration import *

from Configurables import AlgResourcePool,\
                          HiveSlimEventLoopMgr,\
                          HiveSlimEventLoopMgr,\
                          HiveWhiteBoard,\
                          HiveReadAlgorithm,\
                          ForwardSchedulerSvc,\
                          EventClockSvc,\
                          ODINDecodeTool

#-----------------------------------------------------------------
# MetaCfg
simultaneousEvents = 1

#-----------------------------------------------------------------
# WhiteBoard (replacing the one in Brunel)
whiteboard   = HiveWhiteBoard("EventDataSvc",
                              ForceLeaves        = True,
                              RootCLID           =    1,
                              EnableFaultHandler = True,
                              EventSlots = simultaneousEvents)

#-----------------------------------------------------------------
# Scheduler
# configured to be sequential: 1 evt in flight, no deps, 1 algo in flight
scheduler = ForwardSchedulerSvc(MaxEventsInFlight = simultaneousEvents,
                                MaxAlgosInFlight = 1,
                                OutputLevel=WARNING,
                                AlgosDependencies = [])

#-----------------------------------------------------------------
# Resource Pool
AlgResourcePool(OutputLevel = DEBUG,
                DoHacks=True)

#-----------------------------------------------------------------
# Event Loop Manager
slimeventloopmgr = HiveSlimEventLoopMgr(SchedulerName="ForwardSchedulerSvc",
                                        OutputLevel=INFO)
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
       EvtMax=10,
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

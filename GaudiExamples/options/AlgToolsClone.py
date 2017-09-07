###############################################################
# Job options file
#==============================================================
from Gaudi.Configuration import *
from Configurables import MyAlgorithm, MyGaudiAlgorithm,ToolSvc,AlgResourcePool,HiveSlimEventLoopMgr,HiveWhiteBoard

from Configurables import GaudiExamplesCommonConf
GaudiExamplesCommonConf()

myalg = MyAlgorithm('MyAlg',
                    PrivateToolsOnly = True,
                    Cardinality = 20)

ToolSvc(OutputLevel = INFO )

algResourcePool = AlgResourcePool(OutputLevel = INFO)
slimeventloopmgr = HiveSlimEventLoopMgr(SchedulerName="AvalancheSchedulerSvc", OutputLevel=INFO)
whiteboard   = HiveWhiteBoard("EventDataSvc")

ApplicationMgr( EvtMax = 1,
                EvtSel = 'NONE',
                HistogramPersistency = 'NONE',
                EventLoop = slimeventloopmgr,
                ExtSvc = [algResourcePool,whiteboard],
                TopAlg = [myalg] )



from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveEventLoopMgr, HiveTestAlgorithm

evtslots = 10

whiteboard   = HiveWhiteBoard("EventDataSvc",
                              EventSlots = evtslots)
eventloopmgr = HiveEventLoopMgr(MaxEventsParallel = evtslots,
                                MaxAlgosParallel  = 20,
                                NumThreads = 5,
                                AlgosDependencies = [[],['a1'],['a1'],['a2','a3']])

a1 = HiveTestAlgorithm("A1", Outputs = ['/Event/a1'])
a2 = HiveTestAlgorithm("A2", Inputs = ['/Event/a1'],
                             Outputs = ['/Event/a2'])
a3 = HiveTestAlgorithm("A3", Inputs = ['/Event/a1'],
                             Outputs = ['/Event/a3'])
a4 = HiveTestAlgorithm("A4", Inputs = ['/Event/a2','/Event/a3'],
                             Outputs = ['/Event/a4'])

ApplicationMgr( EvtMax = 500,
                EvtSel = 'NONE',
                ExtSvc =[whiteboard],
                EventLoop = eventloopmgr,
                TopAlg = [a1,a2,a3,a4] )

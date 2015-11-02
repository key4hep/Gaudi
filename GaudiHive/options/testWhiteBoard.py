from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveEventLoopMgr, HiveTestAlgorithm

evtslots = 10

whiteboard   = HiveWhiteBoard("EventDataSvc",
                              EventSlots = evtslots)
eventloopmgr = HiveEventLoopMgr(MaxEventsParallel = evtslots,
                                MaxAlgosParallel  = 20,
                                NumThreads = 8,
                                AlgosDependencies = [[],['a1'],['a1'],['a2','a3']])

a1 = HiveTestAlgorithm("A1", Output = ['/Event/a1'])
a2 = HiveTestAlgorithm("A2", Input = ['/Event/a1'],
                             Output = ['/Event/a2'])
a3 = HiveTestAlgorithm("A3", Input = ['/Event/a1'],
                             Output = ['/Event/a3'])
a4 = HiveTestAlgorithm("A4", Input = ['/Event/a2','/Event/a3'],
                             Output = ['/Event/a4'])

ApplicationMgr( EvtMax = 100,
                EvtSel = 'NONE',
                ExtSvc =[whiteboard],
                EventLoop = eventloopmgr,
                TopAlg = [a1,a2,a3,a4] )

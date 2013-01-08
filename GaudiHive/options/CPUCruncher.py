#!/usr/bin/env gaudirun.py

from Gaudi.Configuration import *
from Configurables import HiveWhiteBoard, HiveEventLoopMgr, CPUCruncher

evtslots = 10

whiteboard   = HiveWhiteBoard("EventDataSvc",
                              EventSlots = evtslots)
                              
eventloopmgr = HiveEventLoopMgr(MaxEventsParallel = evtslots,
                                MaxAlgosParallel  = 20,
                                NumThreads = 8,
                                AlgosDependencies = [[],['a1'],['a1'],['a2','a3']])

a1 = CPUCruncher("A1", 
                 Outputs = ['/Event/a1'],
                 varRuntime=.1, 
                 avgRuntime=.5 )
a2 = CPUCruncher("A2", 
                 Inputs = ['/Event/a1'],
                 Outputs = ['/Event/a2'])
a3 = CPUCruncher("A3", 
                 Inputs = ['/Event/a1'],
                 Outputs = ['/Event/a3'])
a4 = CPUCruncher("A4", 
                 Inputs = ['/Event/a2','/Event/a3'],
                 Outputs = ['/Event/a4'])

ApplicationMgr( EvtMax = 50,
                EvtSel = 'NONE',
                ExtSvc =[whiteboard],
                EventLoop = eventloopmgr,
                TopAlg = [a1,a2,a3,a4] )

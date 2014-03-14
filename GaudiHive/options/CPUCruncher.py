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
                 varRuntime=.1,
                 avgRuntime=.5,
                 shortCalib = True )
a1.OutputDataObjects.output_0.address = '/Event/a1'

a2 = CPUCruncher("A2")
a2.InputDataObjects.input_0.address = '/Event/a1'
a2.OutputDataObjects.output_0.address = '/Event/a2'

a3 = CPUCruncher("A3")
a3.InputDataObjects.input_0.address = '/Event/a1'
a3.OutputDataObjects.output_0.address = '/Event/a3'

a4 = CPUCruncher("A4")
a4.InputDataObjects.input_0.address = '/Event/a2'
a4.InputDataObjects.input_1.address = '/Event/a3'
a4.OutputDataObjects.output_0.address = '/Event/a4'

ApplicationMgr( EvtMax = 10,
                EvtSel = 'NONE',
                ExtSvc =[whiteboard],
                EventLoop = eventloopmgr,
                TopAlg = [a1,a2,a3,a4] )

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
a1.Outputs.output_0.Path = '/Event/a1'

a2 = CPUCruncher("A2")
a2.Inputs.input_0.Path = '/Event/a1'
a2.Outputs.output_0.Path = '/Event/a2'

a3 = CPUCruncher("A3")
a3.Inputs.input_0.Path = '/Event/a1'
a3.Outputs.output_0.Path = '/Event/a3'

a4 = CPUCruncher("A4")
a4.Inputs.input_0.Path = '/Event/a2'
a4.Inputs.input_1.Path = '/Event/a3'
a4.Outputs.output_0.Path = '/Event/a4'

ApplicationMgr( EvtMax = 10,
                EvtSel = 'NONE',
                ExtSvc =[whiteboard],
                EventLoop = eventloopmgr,
                TopAlg = [a1,a2,a3,a4] )

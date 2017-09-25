###############################################################
# Job options file
#==============================================================

from Gaudi.Configuration import *
from Configurables import ParentAlg,\
    StopperAlg,\
    Prescaler,\
    HelloWorld,\
    TimingAuditor,\
    AlgResourcePool,\
    HiveSlimEventLoopMgr,\
    ForwardSchedulerSvc,\
    HiveWhiteBoard

from Configurables import GaudiExamplesCommonConf
GaudiExamplesCommonConf()

#--------------------------------------------------------------
# Testing Sequencers
#--------------------------------------------------------------
p1 = Prescaler('Prescaler1', PercentPass=50., OutputLevel=INFO)
p2 = Prescaler('Prescaler2', PercentPass=10., OutputLevel=INFO)
h = HelloWorld(OutputLevel=INFO)
c1 = EventCounter('Counter1')
c2 = EventCounter('Counter2')
s1 = Sequencer('Sequence1', Members=[p1, h, c1])
s2 = Sequencer('Sequence2', Members=[p2, h, c2])
top = Sequencer('TopSequence', Members=[s1, s2], StopOverride=True)

#-----------------------------------------------------------------
# Testing the new GaudiSequencer
#-----------------------------------------------------------------
sand = GaudiSequencer('ANDSequence',
                      Members=[HelloWorld('ANDHelloWorld'),
                               EventCounter('ANDCounter')],
                      MeasureTime=1)
sor = GaudiSequencer('ORSequence',
                     Members=[HelloWorld('ORHelloWorld'),
                              EventCounter('ORCounter')],
                     MeasureTime=1,
                     ModeOR=1)

#-----------------------------------------------------------------

topAlgList = [ParentAlg(), StopperAlg(StopCount=20), top, sand, sor]

#-----------------------------------------------------------------
# Resource Pool
AlgResourcePool(OutputLevel=INFO)

#-----------------------------------------------------------------
# Event Loop Manager
slimeventloopmgr = HiveSlimEventLoopMgr(OutputLevel=INFO)

#-----------------------------------------------------------------
# Scheduler
# configured to be sequential: 1 evt in flight, no deps, 1 algo in flight
scheduler = ForwardSchedulerSvc(MaxEventsInFlight=1,
                                MaxAlgosInFlight=1,
                                OutputLevel=WARNING,
                                AlgosDependencies=[])

#-----------------------------------------------------------------
# WhiteBoard
whiteboard = HiveWhiteBoard("EventDataSvc",
                            EventSlots=1)

#-----------------------------------------------------------------

ApplicationMgr(TopAlg=topAlgList,
               EvtMax=1,
               EvtSel='NONE',  # do not use any event input
               ExtSvc=['ToolSvc', 'AuditorSvc', whiteboard],
               EventLoop=slimeventloopmgr,
               MessageSvcType="InertMessageSvc")

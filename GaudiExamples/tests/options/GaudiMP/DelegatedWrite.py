from Configurables import (ApplicationMgr, EventDataSvc,
                           RecordOutputStream, ReplayOutputStream,
                           GaudiSequencer,
                           GaudiTesting__OddEventsFilter as OddEvents,
                           GaudiTesting__EvenEventsFilter as EvenEvents,
                           SubAlg as EmptyAlg)

outDelegate = ReplayOutputStream()
outDelegate.OutputStreams = [EmptyAlg('Stream1'), EmptyAlg('Stream2')]

oddEvtSelect = GaudiSequencer('OddEventsSelection')
oddEvtSelect.Members = [OddEvents('OddEvents'),
                        RecordOutputStream('Rec1', OutputStreamName='Stream1')]

evenEvtSelect = GaudiSequencer('EvenEventsSelection')
evenEvtSelect.Members = [EvenEvents('EvenEvents'),
                         RecordOutputStream('Rec2', OutputStreamName='Stream2')]

app = ApplicationMgr(EvtSel='NONE', EvtMax=4)
app.TopAlg = [EmptyAlg("EventInit"), evenEvtSelect, oddEvtSelect]
app.OutStream = [outDelegate]

EventDataSvc(ForceLeaves=True)

#from Gaudi.Configuration import VERBOSE
#from Configurables import MessageSvc
#MessageSvc(OutputLevel=VERBOSE)

# Standard app configuration
from Configurables import (ApplicationMgr,
                           GaudiSequencer,
                           GaudiTesting__OddEventsFilter as OddEvents,
                           GaudiTesting__EvenEventsFilter as EvenEvents,
                           SubAlg as EmptyAlg,
                           OutputStream, InputCopyStream)

oddEvtSelect = GaudiSequencer('OddEventsSelection')
oddEvtSelect.Members = [OddEvents('OddEvents'),
                        OutputStream('Stream1')]

evenEvtSelect = GaudiSequencer('EvenEventsSelection')
evenEvtSelect.Members = [EvenEvents('EvenEvents'),
                         InputCopyStream('Stream2')]

app = ApplicationMgr(EvtSel='NONE', EvtMax=4)
app.TopAlg = [EmptyAlg("EventInit"), evenEvtSelect, oddEvtSelect]


# Extension used to enforce deferring
from Configurables import (ApplicationMgr, EventDataSvc,
                           ReplayOutputStream,
                           SubAlg as EmptyAlg)

app = ApplicationMgr()
app.AlgTypeAliases['OutputStream'] = 'RecordOutputStream'
app.AlgTypeAliases['InputCopyStream'] = 'RecordOutputStream'
EventDataSvc(ForceLeaves=True)

outDelegate = ReplayOutputStream()
outDelegate.OutputStreams = [EmptyAlg('Deferred:Stream1'),
                             EmptyAlg('Deferred:Stream2')]
app.OutStream = [outDelegate]

#from Gaudi.Configuration import VERBOSE
#from Configurables import MessageSvc
#MessageSvc(OutputLevel=VERBOSE)

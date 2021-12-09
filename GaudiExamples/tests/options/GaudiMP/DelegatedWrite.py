#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from Configurables import ApplicationMgr, EventDataSvc, GaudiSequencer
from Configurables import GaudiTesting__EvenEventsFilter as EvenEvents
from Configurables import GaudiTesting__OddEventsFilter as OddEvents
from Configurables import RecordOutputStream, ReplayOutputStream
from Configurables import SubAlg as EmptyAlg

outDelegate = ReplayOutputStream()
outDelegate.OutputStreams = [EmptyAlg("Stream1"), EmptyAlg("Stream2")]

oddEvtSelect = GaudiSequencer("OddEventsSelection")
oddEvtSelect.Members = [
    OddEvents("OddEvents"),
    RecordOutputStream("Rec1", OutputStreamName="Stream1"),
]

evenEvtSelect = GaudiSequencer("EvenEventsSelection")
evenEvtSelect.Members = [
    EvenEvents("EvenEvents"),
    RecordOutputStream("Rec2", OutputStreamName="Stream2"),
]

app = ApplicationMgr(EvtSel="NONE", EvtMax=4)
app.TopAlg = [EmptyAlg("EventInit"), evenEvtSelect, oddEvtSelect]
app.OutStream = [outDelegate]

EventDataSvc(ForceLeaves=True)

# from Gaudi.Configuration import VERBOSE
# from Configurables import MessageSvc
# MessageSvc(OutputLevel=VERBOSE)

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
# Standard app configuration
from Configurables import ApplicationMgr, Gaudi__Sequencer
from Configurables import GaudiTesting__EvenEventsFilter as EvenEvents
from Configurables import GaudiTesting__OddEventsFilter as OddEvents
from Configurables import InputCopyStream, OutputStream
from Configurables import SubAlg as EmptyAlg

oddEvtSelect = Gaudi__Sequencer("OddEventsSelection")
oddEvtSelect.Members = [OddEvents("OddEvents"), OutputStream("Stream1")]

evenEvtSelect = Gaudi__Sequencer("EvenEventsSelection")
evenEvtSelect.Members = [EvenEvents("EvenEvents"), InputCopyStream("Stream2")]

app = ApplicationMgr(EvtSel="NONE", EvtMax=4)
app.TopAlg = [EmptyAlg("EventInit"), evenEvtSelect, oddEvtSelect]

# Extension used to enforce deferring
from Configurables import ApplicationMgr, EventDataSvc, ReplayOutputStream
from Configurables import SubAlg as EmptyAlg

app = ApplicationMgr()
app.AlgTypeAliases["OutputStream"] = "RecordOutputStream"
app.AlgTypeAliases["InputCopyStream"] = "RecordOutputStream"
EventDataSvc(ForceLeaves=True)

outDelegate = ReplayOutputStream()
outDelegate.OutputStreams = [EmptyAlg("Deferred:Stream1"), EmptyAlg("Deferred:Stream2")]
app.OutStream = [outDelegate]

# from Gaudi.Configuration import VERBOSE
# from Configurables import MessageSvc
# MessageSvc(OutputLevel=VERBOSE)

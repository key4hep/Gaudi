#####################################################################################
# (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import pytest
from GaudiTesting import GaudiExeTest


# Note: we need at least 3 events to have multiple event in flight as
# the first one is always run by itself
def config(event_slots=2, events=3, threads=6):
    """
    Run a multithreaded Gaudi with a few algorithms that take a lot
    of time to force detection of slow events.

    See https://gitlab.cern.ch/gaudi/Gaudi/-/issues/287
    """
    from GaudiConfig2 import Configurables as C

    from Gaudi.Configuration import DEBUG, WARNING

    whiteboard = C.HiveWhiteBoard("EventDataSvc", EventSlots=event_slots)
    scheduler = C.AvalancheSchedulerSvc(
        ThreadPoolSize=threads,
        OutputLevel=WARNING,
    )
    slimeventloopmgr = C.HiveSlimEventLoopMgr(
        SchedulerName=scheduler.name, OutputLevel=DEBUG
    )

    app = C.ApplicationMgr(
        EvtMax=events,
        EvtSel="NONE",
        MessageSvcType="InertMessageSvc",
        EventLoop=slimeventloopmgr.toStringProperty(),
    )
    app.TopAlg = [
        C.Gaudi.EventWatchdogAlg(EventTimeout=2),
        C.GaudiTesting.SleepyAlg("Sleepy7", SleepTime=7),
        C.GaudiTesting.SleepyAlg("Sleepy3", SleepTime=3),
        C.GaudiTesting.SleepyAlg("Sleepy1", SleepTime=1),
    ]
    app.ExtSvc = [whiteboard]

    return [app] + list(app.TopAlg) + list(app.ExtSvc) + [scheduler, slimeventloopmgr]


class Test(GaudiExeTest):
    command = ["gaudirun.py", f"{__file__}:config"]

    expected_messages = [
        b"More than 2s since the beginning of the event (s: 0  e: 0)",
        b"More than 2s since the beginning of the event (s: 1  e: 1)",
        b"More than 2s since the beginning of the event (s: 0  e: 2)",
        b"An event (s: 0  e: 0) took 7.",
        b"An event (s: 1  e: 1) took 7.",
        b"An event (s: 0  e: 2) took 7.",
    ]

    @pytest.mark.parametrize("message", expected_messages)
    def test_stdout(self, stdout, message):
        assert message in stdout

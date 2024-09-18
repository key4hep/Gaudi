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
    of time to force detection of slow events, print debug information
    and kill the job.

    See https://gitlab.cern.ch/gaudi/Gaudi/-/issues/287
    """
    from GaudiConfig2 import Configurables as C

    from Gaudi.Configuration import DEBUG, INFO

    whiteboard = C.HiveWhiteBoard("EventDataSvc", EventSlots=event_slots)
    scheduler = C.AvalancheSchedulerSvc(
        ThreadPoolSize=threads,
        OutputLevel=INFO,  # AvalancheSchedulerSvc dumps the state at INFO level
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
        C.Gaudi.EventWatchdogAlg(EventTimeout=2, StackTrace=True, AbortOnTimeout=True),
        C.GaudiTesting.SleepyAlg("Sleepy7", SleepTime=7),
        C.GaudiTesting.SleepyAlg("Sleepy3", SleepTime=3),
        C.GaudiTesting.SleepyAlg("Sleepy1", SleepTime=1),
    ]
    app.ExtSvc = [whiteboard]

    return [app] + list(app.TopAlg) + list(app.ExtSvc) + [scheduler, slimeventloopmgr]


class Test(GaudiExeTest):
    command = ["gaudirun.py", f"{__file__}:config"]
    returncode = -3

    expected_messages = [
        b"More than 2s since the beginning of the event (s: 0  e: 0)",
        b"Dumping scheduler state",
        b"[ slot: 0, event: 0 ]:",
    ]

    @pytest.mark.parametrize("message", expected_messages)
    def test_stdout(self, stdout, message):
        assert message in stdout

    def test_stderr(self, stderr):
        assert b"=== Stalled event: current stack trace (s: 0  e: 0) ===" in stderr

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
from GaudiTests import run_gaudi


# Note: we need at least 3 events to have multiple event in flight as
# the first one is always run by itself
def config(event_slots=2, events=3, threads=6):
    """
    Run a multithreaded Gaudi with a few algorithms that take a lot
    of time to force detection of slow events.

    See https://gitlab.cern.ch/gaudi/Gaudi/-/issues/287
    """
    from Gaudi.Configuration import DEBUG, WARNING
    from GaudiConfig2 import Configurables as C

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


def test():
    proc = run_gaudi(f"{__file__}:config", capture_output=True, text=True)
    assert proc.returncode == 0

    expected_messages = [
        "More than 2s since the beginning of the event (s: 0  e: 0)",
        "More than 2s since the beginning of the event (s: 1  e: 1)",
        "More than 2s since the beginning of the event (s: 0  e: 2)",
        "An event (s: 0  e: 0) took 7.",
        "An event (s: 1  e: 1) took 7.",
        "An event (s: 0  e: 2) took 7.",
    ]
    missing = [message for message in expected_messages if message not in proc.stdout]
    assert not missing

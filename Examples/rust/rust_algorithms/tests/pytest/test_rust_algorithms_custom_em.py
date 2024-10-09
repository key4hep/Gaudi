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


def config(events=3):
    from GaudiConfig2 import Configurables as C
    from GaudiConfig2.Configurables.Gaudi.Examples import RustAlgorithms as R
    from GaudiConfig2.Configurables.Gaudi.Examples import TestAlgorithms as E

    make_point_1 = R.PointProducer(
        "MakePoint1", OutputLocation="/Event/Point1", X=8, Y=7, Z=6
    )
    make_point_2 = R.PointProducer(
        "MakePoint2", OutputLocation="/Event/Point2", X=1, Y=2, Z=3
    )

    diff_points = R.PointsDiff(
        "DiffPoints", InputLocation1="/Event/Point1", InputLocation2="/Event/Point2"
    )

    vec_prod = E.TestVectorProducer(
        "TestVectorProducer", OutputLocation="/Event/Vector"
    )
    vec_cons = R.TestVectorConsumer("TestVectorConsumer", InputLocation="/Event/Vector")

    app = C.ApplicationMgr(
        TopAlg=[make_point_1, make_point_2, diff_points, vec_prod, vec_cons],
        EvtMax=events,
        EvtSel="NONE",
    )

    return [app] + list(app.TopAlg)


def config_mt(events=3, event_slots=2, threads=6):
    from GaudiConfig2 import Configurables as C

    from Gaudi.Configuration import DEBUG, WARNING

    conf = config(events=events)

    whiteboard = C.HiveWhiteBoard("EventDataSvc", EventSlots=event_slots)
    scheduler = C.AvalancheSchedulerSvc(
        ThreadPoolSize=threads,
        OutputLevel=WARNING,
    )
    slimeventloopmgr = C.HiveSlimEventLoopMgr(
        SchedulerName=scheduler.name, OutputLevel=DEBUG
    )

    app = next(c for c in conf if c.name == "ApplicationMgr")
    app.MessageSvcType = "InertMessageSvc"
    app.EventLoop = slimeventloopmgr.toStringProperty()
    app.ExtSvc = [whiteboard]

    conf.extend([whiteboard, scheduler, slimeventloopmgr])

    return conf


def get_alg_messages(stdout: bytes, alg_name: str, level: str = "INFO") -> list[str]:
    return [
        line.split(f"{level} ", 1)[1].strip()
        for line in stdout.decode().splitlines()
        if line.startswith(alg_name) and level in line
    ]


class Test(GaudiExeTest):
    """
    Test a chain of Rust and C++ algorithms that pass simple data to each other.
    """

    command = ["gaudirun.py", f"{__file__}:config"]

    @pytest.mark.parametrize(
        "alg,messages",
        [
            (
                "MakePoint1",
                [
                    "executing MakePoint1, storing Point { x: 8.0, y: 7.0, z: 6.0 } into /Event/Point1 (Rust)"
                ],
            ),
            (
                "MakePoint2",
                [
                    "executing MakePoint2, storing Point { x: 1.0, y: 2.0, z: 3.0 } into /Event/Point2 (Rust)"
                ],
            ),
            (
                "DiffPoints",
                [
                    "Point { x: 8.0, y: 7.0, z: 6.0 } - Point { x: 1.0, y: 2.0, z: 3.0 } = Point { x: 7.0, y: 5.0, z: 3.0 }"
                ],
            ),
            (
                "TestVectorProducer",
                ["putting vector (1, 2, 3) into /Event/Vector"],
            ),
            (
                "TestVectorConsumer",
                ["got vector from /Event/Vector: (1.0, 2.0, 3.0)"],
            ),
        ],
    )
    def test_stdout(self, stdout: bytes, alg: str, messages: list[str]) -> None:
        cfg = config()
        app = next(c for c in cfg if c.name == "ApplicationMgr")
        evt_max = app.EvtMax

        alg_messages = get_alg_messages(stdout, alg)

        assert alg_messages == messages * evt_max


class TestMT(Test):
    """
    Test a chain of Rust and C++ algorithms that pass simple data to each other
    in a multi-threaded job.
    """

    command = ["gaudirun.py", f"{__file__}:config_mt"]

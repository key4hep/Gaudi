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
    from GaudiConfig2.Configurables.Gaudi import TestSuite as T

    i_prod = T.IntDataProducer("IntDataProducer", OutputLocation="/Event/Int")
    i2f = T.IntToFloatData(
        "IntToFloatData", InputLocation="/Event/Int", OutputLocation="/Event/Float"
    )
    f_consumer = T.FloatDataConsumer("FloatDataConsumer", InputLocation="/Event/Float")

    app = C.ApplicationMgr(
        TopAlg=[i_prod, i2f, f_consumer],
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
                "IntDataProducer",
                ["executing IntDataProducer, storing 7 into /Event/Int"],
            ),
            (
                "IntToFloatData",
                ["Converting: 7 from /Event/Int and storing it into /Event/Float"],
            ),
            ("FloatDataConsumer", ["executing FloatDataConsumer: 7"]),
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

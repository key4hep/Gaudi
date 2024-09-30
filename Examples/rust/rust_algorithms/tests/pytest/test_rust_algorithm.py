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
from GaudiTesting import GaudiExeTest


def config():
    from GaudiConfig2 import Configurables as C
    from GaudiConfig2.Configurables.Gaudi import Examples as E

    rust_alg = E.MyRustCountingAlg("MyRustCountingAlg")

    app = C.ApplicationMgr(
        TopAlg=[rust_alg],
        EvtMax=3,
        EvtSel="NONE",
    )

    return [app, rust_alg]


class TestRustInvocation(GaudiExeTest):
    command = ["gaudirun.py", f"{__file__}:config"]

    def test_stdout(self, stdout):
        # extract the number of events processed from the options
        cfg = config()
        app = next(c for c in cfg if c.name == "ApplicationMgr")
        evt_max = app.EvtMax

        # get only the message part of RustyAlg "INFO" lines
        alg_messages = [
            line.split("INFO", 1)[1].strip()
            for line in stdout.decode().splitlines()
            if line.startswith("MyRustCountingAlg") and "INFO" in line
        ]

        expected = ["Initialize MyRustCountingAlg (Rust)"]
        expected.extend(f"counted {i} events" for i in range(1, evt_max + 1))

        assert alg_messages == expected

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


def config():
    from GaudiConfig2 import Configurables as C
    from GaudiConfig2.Configurables.Gaudi import Examples as E

    rust_alg = E.MyRustCountingAlg("MyRustCountingAlg")
    cpp_alg = E.MyCppCountingAlg("MyCppCountingAlg")

    app = C.ApplicationMgr(
        TopAlg=[rust_alg, cpp_alg],
        EvtMax=3,
        EvtSel="NONE",
    )

    return [app, rust_alg, cpp_alg]


class TestRustInvocation(GaudiExeTest):
    command = ["gaudirun.py", f"{__file__}:config"]

    @pytest.mark.parametrize("language", ["C++", "Rust"])
    def test_stdout(self, stdout, language):
        # extract the number of events processed from the options
        cfg = config()
        app = next(c for c in cfg if c.name == "ApplicationMgr")
        evt_max = app.EvtMax

        alg_name = "MyCppCountingAlg" if language == "C++" else "MyRustCountingAlg"

        # get only the message part of RustyAlg "INFO" lines
        alg_messages = [
            line.split("INFO", 1)[1].strip()
            for line in stdout.decode().splitlines()
            if line.startswith(alg_name) and "INFO" in line
        ]

        expected = [f"Initialize {alg_name} ({language:4})"]
        expected.extend(f"counted {i} events" for i in range(1, evt_max + 1))
        expected.append(f"Finalize {alg_name}: count = {evt_max}")

        assert alg_messages == expected

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

    cxx_alg = E.KVSTestAlg("KVSTestC++")
    cxx_alg.KVStore = E.Cpp.KeyValueStore("KVStoreC++")

    rust_alg = E.KVSTestAlg("KVSTestRust")
    rust_alg.KVStore = E.Rust.KeyValueStore("KVStoreRust")

    app = C.ApplicationMgr(
        TopAlg=[cxx_alg, rust_alg],
        EvtMax=3,
        EvtSel="NONE",
    )

    return [app, cxx_alg, cxx_alg.KVStore, rust_alg, rust_alg.KVStore]


class TestRustInvocation(GaudiExeTest):
    command = ["gaudirun.py", f"{__file__}:config"]

    @pytest.mark.parametrize("language", ["C++", "Rust"])
    def test_stdout(self, stdout, language):
        # extract the number of events processed from the options
        cfg = config()
        app = next(c for c in cfg if c.name == "ApplicationMgr")
        evt_max = app.EvtMax

        # get only the message part of RustyAlg "INFO" lines
        alg_messages = [
            line.split("INFO", 1)[1].strip()
            for line in stdout.decode().splitlines()
            if (
                line.startswith(f"KVSTest{language}")
                or line.startswith(f"KVStore{language}")
            )
            and "INFO" in line
        ]

        expected = [f"Initialize KVStore{language} ({language})"]
        expected.extend(
            [
                f"entering KVSTest{language}::execute()",
                "abc -> abc-abc",
                f"leaving KVSTest{language}::execute()",
            ]
            * evt_max
        )

        assert alg_messages == expected

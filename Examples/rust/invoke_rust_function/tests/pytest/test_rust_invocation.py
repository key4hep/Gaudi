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


class TestRustInvocation(GaudiExeTest):
    command = ["gaudirun.py", "GaudiRustExamples.invoke_rust_function:config"]

    def test_stdout(self, stdout):
        # extract the number of events processed from the options
        from GaudiRustExamples.invoke_rust_function import config

        cfg = config()
        app = next(c for c in cfg if c.name == "ApplicationMgr")
        evt_max = app.EvtMax

        # get only the message part of RustyAlg "INFO" lines
        alg_messages = [
            line.split("INFO", 1)[1].strip()
            for line in stdout.decode().splitlines()
            if line.startswith("RustyAlg") and "INFO" in line
        ]

        expected = ["RustyAlg::initialize()"]
        for i in range(evt_max):
            expected.extend(
                [
                    "entering RustyAlg::execute()",
                    f"event count -> {i + 1}",
                    "leaving RustyAlg::execute()",
                ]
            )
        expected.append(f"total event count -> {evt_max}")
        assert alg_messages == expected

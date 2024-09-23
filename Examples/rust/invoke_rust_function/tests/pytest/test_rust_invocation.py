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
    command = ["gaudirun.py", "GaudiExamples.invoke_rust_function:config"]

    def test_stdout(self, stdout):
        # get only the message part of RustyAlg "INFO" lines
        rustyalg_messages = [
            line.split("INFO", 1)[1].strip()
            for line in stdout.decode().splitlines()
            if line.startswith("RustyAlg") and "INFO" in line
        ]
        expected = [
            "entering RustyAlg::execute()",
            "some_rust_function returned 42",
            "leaving RustyAlg::execute()",
        ] * 10
        assert rustyalg_messages == expected

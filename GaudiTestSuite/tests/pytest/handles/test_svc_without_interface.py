#####################################################################################
# (c) Copyright 2025 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################

from GaudiTesting import NO_ERROR_MESSAGES, GaudiExeTest


def config():
    from GaudiConfig2 import Configurables as C

    conf = []
    alg = C.Gaudi.TestSuite.UseSvcWithoutInterface("UseSvcWithoutInterface")
    conf.append(alg)

    appMgr = C.ApplicationMgr(
        TopAlg=[alg],
        EvtMax=1,
        EvtSel="NONE",
    )
    conf.append(appMgr)

    msgsvc = C.MessageSvc(Format="% F%25W%S%7W%R%T %0W%M")
    conf.append(msgsvc)

    return conf


class Test(GaudiExeTest):
    command = ["gaudirun.py", f"{__file__}:config"]

    reference = {"messages_count": NO_ERROR_MESSAGES}

    def test_stdout(self, stdout: bytes):
        extracted_lines = [
            line.split(None, 2)
            for line in stdout.splitlines()
            if line.startswith(b"UseSvcWithoutInterface")
            or line.startswith(b"SvcWithoutInterface")
        ]
        expected_lines = [
            [b"UseSvcWithoutInterface", b"INFO", b"initializing..."],
            [b"SvcWithoutInterface", b"INFO", b"initialized"],
            [b"SvcWithoutInterface", b"INFO", b"doing something"],
            [b"UseSvcWithoutInterface", b"INFO", b"initialized"],
        ]
        assert extracted_lines == expected_lines

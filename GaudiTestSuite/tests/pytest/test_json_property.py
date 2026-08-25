#####################################################################################
# (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an intergovernmental organization or     #
# submit itself to any jurisdiction.                                                #
#####################################################################################
import json

from GaudiTesting import GaudiExeTest

PAYLOAD = {
    "enabled": True,
    "items": [1, None, "value"],
    "nested": {"threshold": 2.5},
}


def config():
    from GaudiConfig2 import Configurables as C

    algorithm = C.JSONPropertyAlg(JSON=PAYLOAD)
    return [
        C.ApplicationMgr(EvtMax=0, EvtSel="NONE", TopAlg=[algorithm]),
        algorithm,
    ]


class TestJSONProperty(GaudiExeTest):
    opts_dump = "opts.json"
    command = ["gaudirun.py", f"{__file__}:config", "-o", opts_dump]

    def test_python_value_reaches_job_options(self, cwd):
        with (cwd / self.opts_dump).open() as stream:
            options = json.load(stream)

        assert json.loads(options["JSONPropertyAlg.JSON"]) == PAYLOAD

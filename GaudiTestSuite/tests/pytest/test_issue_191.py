#####################################################################################
# (c) Copyright 2024-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import json

from GaudiTesting import GaudiExeTest


def config():
    from Configurables import ApplicationMgr
    from GaudiConfig2 import Configurables as C

    ApplicationMgr(AppName="Test", EvtMax=10)
    app = C.ApplicationMgr(AppName="Test", EvtMax=10)
    return [app]


class Test(GaudiExeTest):
    opts_dump = "opts.json"
    command = ["gaudirun.py", f"{__file__}:config", "-n", "-o", opts_dump]

    def test_opts_dump(self, cwd):
        opts_path = cwd / self.opts_dump
        assert opts_path.exists()

        with opts_path.open() as f:
            opts = json.load(f)
        assert opts["ApplicationMgr.AppName"] == f'{repr("Test")}'
        assert opts["ApplicationMgr.EvtMax"] == f"{repr(10)}"

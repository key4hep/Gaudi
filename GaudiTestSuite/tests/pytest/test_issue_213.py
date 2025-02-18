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
    # use a ConfigurableUser specialization
    from Configurables import GaudiTestSuiteCommonConf

    GaudiTestSuiteCommonConf(DummyEvents=42)

    # no GaudiConfig2 configurables to add


class Test(GaudiExeTest):
    """
    Run gaudirun.py with a Python option function that instantiates
    a ConfigurableUser specialization.

    See https://gitlab.cern.ch/gaudi/Gaudi/-/issues/213
    """

    opts_dump = "opts.json"
    command = ["gaudirun.py", "-n", "-o", opts_dump, f"{__file__}:config"]

    def test_opts_dump(self, cwd):
        opts_path = cwd / self.opts_dump
        assert opts_path.exists()

        with opts_path.open() as f:
            opts = json.load(f)
        assert opts["ApplicationMgr.EvtMax"] == "42"

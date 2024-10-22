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
from pathlib import Path

from conftest import OptParseTest


class TestTest(OptParseTest):
    "https://gitlab.cern.ch/gaudi/Gaudi/-/issues/188"

    command = [
        "gaudirun.py",
        "--dry-run",
        "--verbose",
        "--all-opts",
        "test_issue_188:config",
    ]
    environment = [
        f"PYTHONPATH={Path(__file__).parent.parent / 'python'}:$PYTHONPATH",
    ]

    def test_options(self, options_dump: dict):
        assert (
            "ApplicationMgr" in options_dump
        ), "ApplicationMgr not in the configuration dump"

        expected = {
            "AppName": "Test",  # from old options
            "AppVersion": "v100r100",  # from GaudiConfig2
            "OutputLevel": 3,  # from default
        }
        # we want to check only a few of the options
        result = {
            key: value
            for key, value in options_dump["ApplicationMgr"].items()
            if key in expected
        }
        assert result == expected

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
import re
from pathlib import Path

from conftest import OptParseTest


class Test(OptParseTest):
    environment = [
        f"JOBOPTSEARCHPATH={Path(__file__).parent.parent / 'pyjobopts'}",
    ]

    command = ["gaudirun.py", "--dry-run", "--verbose", "main.opts"]

    expected_dump = {
        "ApplicationMgr": {
            "TopAlg": ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9"]
        },
        "MessageSvc": {"OutputLevel": 4},
        "myAlg": {"Dict": {"a": 1, "b": 2}},
        "myAlg.myTool": {"Int": 10, "Bool": True},
        "mySvc": {"List": ["a", "c"], "Bool": True},
    }

    def test_warning(self, stdout):
        assert re.search(
            r"WARNING: file '.*test1.opts' already included, ignored", stdout.decode()
        ), "missing expected warning"

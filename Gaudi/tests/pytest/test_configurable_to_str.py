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


class Test(OptParseTest):
    environment = [
        f"JOBOPTSEARCHPATH={Path(__file__).parent.parent / 'pyjobopts'}",
    ]

    command = ["gaudirun.py", "--dry-run", "--verbose", "configurable_to_str.py"]

    expected_dump = {
        "Algorithm1": {"Service": "MySvc/JustAName"},
        "Algorithm2": {"Members": ["HelloWorld", "MyAlg/Algorithm1"]},
        "Algorithm3": {
            "Members": ["MyAlg/Algorithm1", "HelloWorld", "MyAlg/Algorithm2"]
        },
        "JustAName": {"OutputLevel": 1},
        "MessageSvc": {"OutputLevel": 3},
    }

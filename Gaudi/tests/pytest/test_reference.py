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

    command = ["gaudirun.py", "--dry-run", "--verbose", "reference.py"]

    expected_dump = {
        "MyAlg": {
            "Cut": 20.0,
            "File": "C:\\something\\vXrY\\filename",
            "Tool1": "ThisTool",
            "Tool2": "MyToolType/MyToolType",
        },
        "MyAlg.MyTool": {
            "Cut": 20.0,
            "WhichFile": "C:\\something\\vXrY\\filename",
            "WhichTool1": "ThisTool",
            "WhichTool2": "MyToolType/MyToolType",
        },
    }

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

    command = ["gaudirun.py", "--dry-run", "--verbose", "case_sensitivity.py"]

    expected_dump = {
        "MyAlg1": {"AnotherTest": "AValue", "TestProperty": "AValue"},
        "MyAlg2": {"TestProperty": "AValue"},
    }

    def test_stdout(self, stdout):
        all_warnings = [
            line for line in stdout.decode().splitlines() if "warning" in line.lower()
        ]
        expected = [
            "# WARNING: property 'testproperty' was requested for MyAlg1, but the correct spelling is 'TestProperty'",
            "# WARNING: Option 'testproperty' was used for MyAlg2, but the correct spelling is 'TestProperty'",
        ]
        assert all_warnings == expected

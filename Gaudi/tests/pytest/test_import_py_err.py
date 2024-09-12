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

from GaudiTesting import GaudiExeTest


class Test(GaudiExeTest):
    environment = [
        f"JOBOPTSEARCHPATH={Path(__file__).parent.parent / 'pyjobopts'}",
    ]

    command = ["gaudirun.py", "--dry-run", "--verbose", "main_err.py"]
    returncode = 1

    def test_stderr(conf, stderr):
        expected_error = b"AttributeError: Configurable 'MessageSvc' does not have property 'IDontHaveIt'"
        assert expected_error in stderr

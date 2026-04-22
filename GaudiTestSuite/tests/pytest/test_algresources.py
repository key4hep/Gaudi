#####################################################################################
# (c) Copyright 2024-2026 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import re

from GaudiTesting import GaudiExeTest


class TestReEntAlg(GaudiExeTest):
    command = ["gaudirun.py", "../../options/ReEntAlgResources.py"]

    def test_stdout(self, stdout):
        expected = r"ReEntAlg\s*INFO Maximum number of concurrent executions: 2"
        assert re.search(expected, stdout.decode())

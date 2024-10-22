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

from GaudiTesting import GaudiExeTest


class TestTiming(GaudiExeTest):
    command = ["gaudirun.py", "../../options/Timing.py"]

    def test_stdout(self, stdout):
        expected = (
            r"Timing\s+SUCCESS\s+The timing is \(in us\)\s*\n"
            + r"\|\s*\|\s*#\s*\|\s*Total\s*\|\s*Mean\+-RMS\s*\|\s*Min/Max\s*\|\s*\n"
            + r"\|\s*\(1U\)\s*\|\s*\d*\s*\|(?:[-+.\deE /]+\|){3}\s*\n"
            + r"\|\s*\(2U\)\s*\|\s*\d*\s*\|(?:[-+.\deE /]+\|){3}\s*\n"
            + r"\|\s*\(3U\)\s*\|\s*\d*\s*\|(?:[-+.\deE /]+\|){3}\s*\n"
        )

        assert re.search(expected, stdout.decode())

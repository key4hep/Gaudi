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
from GaudiTesting import GaudiExeTest


class Test(GaudiExeTest):
    command = ["gaudirun.py", "../../../options/ConditionsStallTest.py"]
    timeout = 60
    returncode = 1

    def test_stdout(self, stdout):
        expected = b"AlgA (2), w/ decision: UNDEFINED(-1), in state: CONTROLREADY"
        assert expected in stdout, "no signature of conditions stall detection found"

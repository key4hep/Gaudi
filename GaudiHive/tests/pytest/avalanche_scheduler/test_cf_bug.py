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


def filter_out(stdout):
    "remove noise from stdout"
    return "\n".join(
        line for line in stdout.splitlines() if "Popped slot 0" not in line
    )


class Test(GaudiExeTest):
    command = ["gaudirun.py", "-v", "../../../options/CFBugWithEmptyNode.py"]
    timeout = 60

    test_block = GaudiExeTest.find_reference_block(
        """
        AvalancheSchedu...  DEBUG Event 0 finished (slot 0).
        AvalancheSchedu...  DEBUG RootDecisionHub (0), w/ decision: TRUE(1)
          topSeq (1), w/ decision: FALSE(0)
            A1 (2), w/ decision: TRUE(1), in state: EVTACCEPTED
            emptySeq (3), w/ decision: FALSE(0)
            A2 (4), w/ decision: UNDEFINED(-1), in state: INITIAL
        """,
        preprocessor=filter_out,
    )

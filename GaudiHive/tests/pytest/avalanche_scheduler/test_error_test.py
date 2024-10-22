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
    command = ["gaudirun.py", "-v", "../../../options/AvalancheSchedulerErrorTest.py"]
    timeout = 600

    test_block1 = GaudiExeTest.find_reference_block(
        """
        [ slot: 7, event: 7 ]:

        ERROR alg(s): A3
        """
    )

    test_block2 = GaudiExeTest.find_reference_block(
        """
        HiveSlimEventLoopMgr                    ERROR Failed event detected on s: 7  e: 7
        """
    )

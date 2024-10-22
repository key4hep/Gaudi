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


class TestExplicit(GaudiExeTest):
    command = ["gaudirun.py", "-v", "../../options/GaudiMP/DelegatedWrite.py"]

    def filter_out(stdout):
        return "\n".join(
            line
            for line in stdout.splitlines()
            if "EventLoopMgr      SUCCESS Event Number =" not in line
            and " ---> Loop Finished" not in line
        )

    test_check_line = GaudiExeTest.find_reference_block(
        """
        ApplicationMgr       INFO Application Manager Initialized successfully
        ApplicationMgr       INFO Application Manager Started successfully
        EventInit            INFO executing....
        Stream1              INFO executing....
        EventInit            INFO executing....
        Stream2              INFO executing....
        EventInit            INFO executing....
        Stream1              INFO executing....
        EventInit            INFO executing....
        Stream2              INFO executing....
        ApplicationMgr       INFO Application Manager Stopped successfully
        Stream1              INFO finalizing....
        Stream2              INFO finalizing....
        EventInit            INFO finalizing....
        """,
        preprocessor=filter_out,
    )

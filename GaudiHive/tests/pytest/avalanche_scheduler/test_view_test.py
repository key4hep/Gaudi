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
from GaudiTesting import GaudiExeTest


class Test(GaudiExeTest):
    command = ["gaudirun.py", "-v", "../../../options/BasicViewTest.py"]
    timeout = 120
    # Lines can appear in different order so we check them one-by-one
    test_block1 = GaudiExeTest.find_reference_block(
        "A1                    INFO Running in whole event context"
    )
    test_block2 = GaudiExeTest.find_reference_block(
        "A2                    INFO Running in view view0"
    )
    test_block3 = GaudiExeTest.find_reference_block(
        "A2                    INFO Running in view view1"
    )
    test_block4 = GaudiExeTest.find_reference_block(
        "A3                    INFO Running in view view0"
    )
    test_block5 = GaudiExeTest.find_reference_block(
        "A3                    INFO Running in view view1"
    )
    test_block6 = GaudiExeTest.find_reference_block(
        "A4                    INFO Running in whole event context"
    )

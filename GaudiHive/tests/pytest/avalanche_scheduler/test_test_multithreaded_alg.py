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
    command = ["gaudirun.py", "-v", "../../../options/testMultithreadedAlg.py"]
    timeout = 60

    test_block = GaudiExeTest.find_reference_block("""
        A1                  DEBUG CPUCrunch complete in TBB parallel for block 4 to 5
        """)

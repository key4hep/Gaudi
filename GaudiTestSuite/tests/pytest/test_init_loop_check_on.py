#####################################################################################
# (c) Copyright 2024-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiTesting import GaudiExeTest


class TestInitLoopCheckOn(GaudiExeTest):
    command = ["gaudirun.py"]
    environment = [
        "JOBOPTSEARCHPATH=$ENV_PROJECT_SOURCE_DIR/GaudiTestSuite/options:$ENV_PROJECT_SOURCE_DIR/Gaudi/GaudiTestSuite/options"
    ]
    returncode = 1

    options = """
        #include "Common.opts"
        ApplicationMgr.TopAlg = { "LoopAlg" };
        """

    test_check_line = GaudiExeTest.find_reference_block(
        """
        ServiceManager       ERROR Initialization loop detected when creating service "ServiceB"
        """
    )

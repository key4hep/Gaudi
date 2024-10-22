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


class TestCustomAppFromPython(GaudiExeTest):
    command = [
        "gaudirun.py",
        "-v",
        "--application=Test::CustomApp",
        "../../options/CustomAppFromOptions.py",
    ]
    environment = [
        "ROOT_INCLUDE_PATH=$ENV_PROJECT_SOURCE_DIR/GaudiKernel/include:$ENV_PROJECT_SOURCE_DIR/GaudiPluginService/include:$ROOT_INCLUDE_PATH"
    ]

    test_check_line = GaudiExeTest.find_reference_block(
        """
        === Custom Application ===
        """
    )

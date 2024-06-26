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
import pytest
from GaudiTesting import GaudiExeTest, platform_matches


@pytest.mark.skipif(platform_matches(["win32"]), reason="Not supported on Windows")
class TestColorMsg(GaudiExeTest):
    command = ["Gaudi.exe", "../../options/ColorMsg.opts"]
    reference = "refs/ColorMsg.yaml"

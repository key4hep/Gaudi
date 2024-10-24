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
from GaudiTesting import NO_ERROR_MESSAGES, GaudiExeTest


@pytest.mark.ctest_fixture_setup("gauditestsuite.multi_input.prepare_base")
@pytest.mark.shared_cwd("multi_input")
class Test(GaudiExeTest):
    command = ["gaudirun.py", "-v", "../../../options/MultiInput/PrepareBase.py"]
    reference = {"messages_count": NO_ERROR_MESSAGES}

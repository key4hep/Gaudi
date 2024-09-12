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
from GaudiTesting import GaudiExeTest


@pytest.mark.ctest_fixture_required("gauditestsuite.multi_input.prepare_base")
@pytest.mark.ctest_fixture_setup("gauditestsuite.multi_input.prepare_ext")
@pytest.mark.shared_cwd("multi_input")
class Test(GaudiExeTest):
    command = ["gaudirun.py", "-v", "../../../options/MultiInput/PrepareExt.py"]

    def test_stdout(self, stdout):
        self.count_error_lines({"ERROR": 0}, stdout.decode())

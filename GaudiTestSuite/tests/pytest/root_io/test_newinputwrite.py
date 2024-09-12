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


@pytest.mark.ctest_fixture_required("new_write")
@pytest.mark.ctest_fixture_setup("new_input_write")
@pytest.mark.shared_cwd("new_root_io")
class Test(GaudiExeTest):
    command = ["gaudirun.py", "-v", "../../../options/ROOT_IO/NewInputWrite.py"]
    reference = "../refs/ROOT_IO/NewInputWrite.yaml"

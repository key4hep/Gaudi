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


@pytest.mark.ctest_fixture_setup("gauditestsuite.conditional_output.write")
@pytest.mark.shared_cwd("conditional_output")
class Test(GaudiExeTest):
    command = ["gaudirun.py", "-v", "../../../options/ConditionalOutput/Write.py"]
    reference = "../refs/conditional_output/write.yaml"
    environment = [
        "STDOPTS=$ENV_PROJECT_SOURCE_DIR/GaudiTestSuite/options",
        "JOBOPTSEARCHPATH=$ENV_PROJECT_SOURCE_DIR/GaudiTestSuite/tests/qmtest:$ENV_PROJECT_SOURCE_DIR/GaudiTestSuite/options:$ENV_PROJECT_SOURCE_DIR/Gaudi/tests/pyjobopts:$ENV_PROJECT_SOURCE_DIR/Gaudi/tests",
        "PYTHONPATH=$ENV_PROJECT_SOURCE_DIR/GaudiTestSuite/tests/qmtest:$ENV_PROJECT_SOURCE_DIR/Gaudi/tests/python:$PYTHONPATH",
        "GAUDIAPPNAME=",
        "GAUDIAPPVERSION=",
    ]

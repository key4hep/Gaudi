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
from conftest import OptParseTest


class TestGaudi218(OptParseTest):
    """
    https://its.cern.ch/jira/browse/GAUDI-218
    """

    options = """
    MyAlg.MyOpt = "a"; // Test "comment"
    OtherAlg.MyOpt = 1;
    // grep "// STRIP SELECTION" | awk ...
    AnotherAlg.Opt = 1;
    """

    expected_dump = {
        "MyAlg": {"MyOpt": "a"},
        "OtherAlg": {"MyOpt": 1},
        "AnotherAlg": {"Opt": 1},
    }

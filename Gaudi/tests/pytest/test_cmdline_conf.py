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


class Test(OptParseTest):
    command = [
        "gaudirun.py",
        "--dry-run",
        "--verbose",
        "--option",
        "MessageSvc().setFatal.append('AnotherAlg')",
    ]

    def options(self):
        from Gaudi.Configuration import MessageSvc

        MessageSvc().setDebug.append("AnAlgorithm")

    expected_dump = {
        "MessageSvc": {"setDebug": ["AnAlgorithm"], "setFatal": ["AnotherAlg"]}
    }

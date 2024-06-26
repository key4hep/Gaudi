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
from pathlib import Path

from conftest import OptParseTest


class Test(OptParseTest):
    """
    https://its.cern.ch/jira/browse/GAUDI-219
    """

    environment = [
        f"JOBOPTSEARCHPATH={Path(__file__).parent.parent.parent/'pyjobopts'}",
        "TESTOPTS=test1.opts",
    ]

    def options(self):
        from Gaudi.Configuration import importOptions

        importOptions("$TESTOPTS")

    expected_dump = {
        "MessageSvc": {"OutputLevel": 2},
        "myAlg.myTool": {"Bool": True, "Int": 10},
    }

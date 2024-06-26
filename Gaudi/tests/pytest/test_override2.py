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
    environment = [
        f"JOBOPTSEARCHPATH={Path(__file__).parent.parent / 'pyjobopts'}",
    ]

    def options(self):
        from Gaudi.Configuration import MessageSvc, importOptions

        MessageSvc(OutputLevel=123)
        importOptions("override.opts")
        MessageSvc(OutputLevel=123)

    def test_options(self, options_dump):
        assert options_dump["MessageSvc"]["OutputLevel"] == 123

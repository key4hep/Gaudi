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
    """
    https://its.cern.ch/jira/browse/GAUDI-241
    """

    def options(self):
        from Configurables import DataStreamTool as MyTool

        from Gaudi.Configuration import INFO

        pub = MyTool("Public")
        pub.addTool(MyTool, "Private")
        pub.Private.OutputLevel = INFO

    expected_dump = {"ToolSvc.Public.Private": {"OutputLevel": 3}}

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
from GaudiTesting import GaudiExeTest


class TestGaudi1174(GaudiExeTest):
    command = ["gaudirun.py", "../../../options/AlgTools.py"]

    def options(self):
        from Configurables import MessageSvc

        MessageSvc(countInactive=True)

    test_check_line = GaudiExeTest.find_reference_block(
        """
        Listing sources of Unprotected and Unseen messages
        =======================================================
         Message Source                  |   Level |    Count
        ---------------------------------+---------+-----------
         MyAlg.MyTool                    |   DEBUG |       10
         MyAlg.PrivToolHandle            |   DEBUG |       20
         MyAlg.ToolWithName              |   DEBUG |       10
         TestToolAlg                     |   DEBUG |       13
         ToolSvc.ConstGenericToolHandle  |   DEBUG |       30
         ToolSvc.MyTool                  |   DEBUG |       10
         ToolSvc.PubToolHandle           |   DEBUG |       20
         ToolSvc.TestPubToolHandle       |   DEBUG |       20
        =======================================================
        """
    )

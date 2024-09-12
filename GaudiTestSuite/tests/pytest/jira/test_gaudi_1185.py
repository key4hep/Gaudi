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


class TestGaudi1185(GaudiExeTest):
    command = ["gaudirun.py"]

    def options(self):
        from Configurables import ApplicationMgr, MessageSvc
        from Configurables import GaudiTesting__JIRA__GAUDI_1185 as GAUDI_1185

        from Gaudi.Configuration import INFO, WARNING

        MessageSvc(OutputLevel=WARNING)
        a = GAUDI_1185("Implicit")
        b = GAUDI_1185("Explicit")
        b.OutputLevel = INFO
        ApplicationMgr(TopAlg=[a, b], EvtSel="NONE", EvtMax=1)

    test_check_line = GaudiExeTest.find_reference_block(
        """
        ApplicationMgr       INFO Application Manager Configured successfully
        Implicit          SUCCESS Initial msgLevel(): 4
        Implicit          SUCCESS Current msgLevel(): 4
        Explicit          SUCCESS Initial msgLevel(): 3
        Explicit          SUCCESS Current msgLevel(): 3
        """
    )

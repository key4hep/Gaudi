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


class TestCustomProperties(GaudiExeTest):
    command = ["gaudirun.py", "-v"]

    def options(self):
        # Options of the test job
        from Configurables import ApplicationMgr
        from Configurables import Gaudi__TestSuite__CustomPropertiesAlg as CPA

        from Gaudi.Configuration import DEBUG

        alg = CPA("CPA")
        alg.UnorderedMap["hello"] = "world"
        alg.UnorderedMap["abc"] = "123"
        alg.PropertiesPrint = True

        ApplicationMgr(TopAlg=[alg], OutputLevel=DEBUG)

    test_check_line = GaudiExeTest.find_reference_block(
        """
        CPA                 DEBUG Property ['Name': Value] =  'UnorderedMap':{'abc': '123', 'hello': 'world'}
        """
    )

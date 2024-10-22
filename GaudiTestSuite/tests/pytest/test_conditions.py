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


class TestConditionsAccess(GaudiExeTest):
    command = ["gaudirun.py"]
    reference = "refs/ConditionsAccess.yaml"

    def options(self):
        from Configurables import ApplicationMgr
        from Configurables import Gaudi__TestSuite__Conditions__UserAlg as UA

        ApplicationMgr(
            EvtSel="NONE",
            EvtMax=5,
            TopAlg=[
                UA("ConditionsUser1"),
                UA("ConditionsUser2", MyCondition="another/key"),
            ],
        )

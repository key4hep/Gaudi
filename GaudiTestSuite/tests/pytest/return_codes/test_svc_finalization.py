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


class TestSvcFinalization(GaudiExeTest):
    command = ["gaudirun.py"]
    returncode = 11

    def options(self):
        from Configurables import ApplicationMgr, GaudiTesting__FailingSvc

        app = ApplicationMgr(  # noqa: F841
            TopAlg=[],
            ExtSvc=[GaudiTesting__FailingSvc(Transition="finalize", Mode="failure")],
            EvtSel="NONE",
            EvtMax=5,
        )

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


class TestAnyData(GaudiExeTest):
    command = ["gaudirun.py"]
    reference = "refs/anydata.yaml"

    def options(self):
        from Configurables import (
            AnyDataGetAlgorithm_Int,
            AnyDataGetAlgorithm_VectorInt,
            AnyDataPutAlgorithm,
            ApplicationMgr,
            EvtDataSvc,
        )

        EvtDataSvc("EventDataSvc").RootCLID = 1
        EvtDataSvc("EventDataSvc").RootName = "/Event"
        EvtDataSvc("EventDataSvc").ForceLeaves = True

        pa = AnyDataPutAlgorithm(Location="TestAnyData")
        ga1 = AnyDataGetAlgorithm_Int(Location="TestAnyData/One")
        ga2 = AnyDataGetAlgorithm_VectorInt(Location="TestAnyData/Two")
        app = ApplicationMgr(TopAlg=[pa, ga1, ga2], EvtSel="NONE", EvtMax=2)  # noqa: F841

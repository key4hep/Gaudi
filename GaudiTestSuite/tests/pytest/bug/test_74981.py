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


class Test74981(GaudiExeTest):
    """
    https://its.cern.ch/jira/browse/GAUDI-364
    """

    command = ["gaudirun.py"]
    returncode = 1

    def options(self):
        from Configurables import ApplicationMgr, DataOnDemandSvc, EventDataSvc
        from Configurables import GaudiTesting__GetDataObjectAlg as GetDataObjectAlg

        dod = DataOnDemandSvc(
            UsePreceedingPath=True,
            AlgMap={"MissingObject": "NoAlgorithm"},
            PreInitialize=True,
        )

        alg = GetDataObjectAlg()
        alg.Paths = ["MissingObject"]

        ApplicationMgr(TopAlg=[alg], ExtSvc=[dod], EvtSel="NONE", EvtMax=1)

        EventDataSvc().EnableFaultHandler = True

    def test_check_line(self, stdout):
        expected = b"ERROR Unable to initialize Service: DataOnDemandSvc"
        assert expected in stdout

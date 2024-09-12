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


class TestInputFile(GaudiExeTest):
    command = ["gaudirun.py", "-v"]
    returncode = 2

    def options(self):
        from Configurables import ApplicationMgr, EventSelector, GaudiPersistency
        from Configurables import GaudiTesting__StopLoopAlg as sla

        GaudiPersistency()

        EventSelector(
            Input=["DATAFILE='PFN:none.dst'  SVC='Gaudi::RootEvtSelector' OPT='READ'"]
        )

        app = ApplicationMgr(TopAlg=[sla(EventCount=3, Mode="failure")], EvtMax=5)  # noqa: F841

    def test_check_line(self, stdout):
        expected = b"/none.dst does not exist"
        assert expected in stdout

#####################################################################################
# (c) Copyright 2024-2026 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiTesting import GaudiExeTest


class TestJobOptionsDump(GaudiExeTest):
    command = ["gaudirun.py"]
    environment = ["JOBOPTSDUMPFILE=start.dump", "JOBOPTSDUMPFILESTOP=stop.dump"]

    def options(self):
        from Configurables import HelloWorld

        from Gaudi.Configuration import ApplicationMgr

        app = ApplicationMgr(EvtSel="NONE", EvtMax=1)

        app.TopAlg = [HelloWorld()]

    test_start_line = GaudiExeTest.find_reference_block(
        """
        JobOptionsSvc         INFO Properties are dumped into "start.dump"
        ApplicationMgr        INFO Application Manager Started successfully
        """
    )
    test_stop_line = GaudiExeTest.find_reference_block(
        """
        JobOptionsSvc         INFO Properties are dumped into "stop.dump"
        ApplicationMgr        INFO Application Manager Stopped successfully
        """
    )

    def test_dumps_exist(self):
        assert (self.cwd / "start.dump").is_file()
        assert (self.cwd / "stop.dump").is_file()

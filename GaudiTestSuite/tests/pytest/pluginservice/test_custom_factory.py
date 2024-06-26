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
import pytest
from GaudiTesting import GaudiExeTest


class TestCustomFactory(GaudiExeTest):
    command = ["gaudirun.py"]

    def options(self):
        from Gaudi.Configuration import ApplicationMgr

        ApplicationMgr(
            TopAlg=["PluginServiceTest::CustomFactoryAlgorithm/CFA"],
            EvtSel="NONE",
            EvtMax=1,
        )

    expected = [
        b"created CustomFactoryAlgorithm at",
        b"CFA                  INFO running",
    ]

    @pytest.mark.parametrize("line", expected)
    def test_check_line(self, line, stdout):
        assert line in stdout

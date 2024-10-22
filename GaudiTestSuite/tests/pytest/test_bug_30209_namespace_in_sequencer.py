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


class TestBug30209(GaudiExeTest):
    command = ["gaudirun.py"]

    def options(self):
        from Configurables import ApplicationMgr, GaudiTestSuite__CounterSvcAlg
        from Configurables import Gaudi__Sequencer as Sequencer

        counter2 = GaudiTestSuite__CounterSvcAlg("Counter2")

        seq = Sequencer(
            "Sequencer",
            Members=[
                GaudiTestSuite__CounterSvcAlg.getType(),
                counter2,
                "GaudiTestSuite::StatSvcAlg:invert",
            ],
        )

        ApplicationMgr(TopAlg=[seq], EvtSel="NONE", EvtMax=1)

    def test_check_line(self, stdout):
        expected = b"Sequencer            INFO Member list: GaudiTestSuite::CounterSvcAlg, GaudiTestSuite::CounterSvcAlg/Counter2, GaudiTestSuite::StatSvcAlg:invert"
        assert expected in stdout

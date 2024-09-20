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
from GaudiTesting.GaudiExeTest import GaudiExeTest


class TestAlgorithmDestructor(GaudiExeTest):
    command = ["gaudirun.py", "-v"]

    def options(self):
        from Configurables import ApplicationMgr
        from Configurables import Gaudi__Sequencer as Sequencer
        from Configurables import GaudiTesting__DestructorCheckAlg as dca

        ApplicationMgr(
            TopAlg=[
                dca("TopAlg"),
                Sequencer("seq1", Members=[dca("SequencerAlg"), dca("SharedAlg")]),
                Sequencer(
                    "seq2",
                    Members=[
                        dca("SharedAlg"),
                        dca("SharedAlg2"),
                    ],
                ),
                Sequencer("seq3", Members=[dca("SharedAlg2")]),
            ],
            EvtSel="NONE",
            EvtMax=2,
        )

    blocks = [
        "Destructor of TopAlg",
        "Destructor of SequencerAlg",
        "Destructor of SharedAlg",
        "Destructor of SharedAlg2",
    ]

    @pytest.mark.parametrize("block", blocks)
    def test_check_line(self, block):
        GaudiExeTest.find_reference_block(block)

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
        # FIXME: drop when we drop GAUDI_ENABLE_GAUDIALG
        import Configurables
        from Configurables import ApplicationMgr
        from Configurables import GaudiTesting__DestructorCheckAlg as dca

        if hasattr(Configurables, "GaudiSequencer"):
            # GaudiAlg available
            Sequencer = Configurables.Sequencer
            GaudiSequencer = Configurables.GaudiSequencer
        else:
            # GaudiAlg not available
            Sequencer = Configurables.Gaudi__Sequencer
            GaudiSequencer = Configurables.Gaudi__Sequencer

        ApplicationMgr(
            TopAlg=[
                dca("TopAlg"),
                Sequencer("seq1", Members=[dca("SequencerAlg"), dca("SharedAlg")]),
                GaudiSequencer(
                    "seq2",
                    Members=[
                        dca("GaudiSequencerAlg"),
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
        "Destructor of GaudiSequencerAlg",
        "Destructor of SharedAlg",
        "Destructor of SharedAlg2",
    ]

    @pytest.mark.parametrize("block", blocks)
    def test_check_line(self, block):
        GaudiExeTest.find_reference_block(block)

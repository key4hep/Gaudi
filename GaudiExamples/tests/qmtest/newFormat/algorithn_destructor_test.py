# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="gaudirun.py"
        self.args=["-v"]
        self.options="""from Gaudi.Configuration import *
from Configurables import GaudiTesting__DestructorCheckAlg as dca;

ApplicationMgr(TopAlg = [ dca("TopAlg"),
                          Sequencer(Members = [dca("SequencerAlg"), dca("SharedAlg")]),
                          GaudiSequencer(Members = [dca("GaudiSequencerAlg"), dca("SharedAlg"), dca("SharedAlg2")]),
                          Sequencer("Sequencer2", Members = [dca("SharedAlg2")]),
                           ],
               EvtSel = "NONE",
               EvtMax = 2)
"""

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        self.findReferenceBlock("Destructor of TopAlg", id = "TopAlgDestructor")
        self.findReferenceBlock("Destructor of SequencerAlg", id = "SequencerAlgDestructor")
        self.findReferenceBlock("Destructor of GaudiSequencerAlg", id = "GaudiSequencerAlgDestructor")
        self.findReferenceBlock("Destructor of SharedAlg", id = "SharedAlgDestructor")
        self.findReferenceBlock("Destructor of SharedAlg2", id = "SharedAlg2Destructor")
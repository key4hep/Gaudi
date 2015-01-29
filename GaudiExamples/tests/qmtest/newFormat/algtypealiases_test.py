# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="gaudirun.py"
        self.reference="refs/AlgTypeAliases.ref"
        self.options="""from Gaudi.Configuration import *

from Configurables import ApplicationMgr, HelloWorld, SubAlg

app = ApplicationMgr(EvtSel='NONE', EvtMax=4)

app.TopAlg = ['HelloWorld/HW1', 'unalias:HelloWorld/HW2' ]

app.AlgTypeAliases['HelloWorld'] = 'SubAlg'"""

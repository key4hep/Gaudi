# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="gaudirun.py"
        self.options="""
#include "Common.opts"
ApplicationMgr.TopAlg = { "LoopAlg" };
ApplicationMgr.InitializationLoopCheck = False;"""

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        self.findReferenceBlock('ApplicationMgr       INFO Application Manager Initialized successfully')
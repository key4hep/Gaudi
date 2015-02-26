# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="gaudirun.py"
        self.exit_code=1
        self.options="""
#include "Common.opts"
ApplicationMgr.TopAlg = { "LoopAlg" };"""

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        self.findReferenceBlock('ServiceManager      ERROR Initialization loop detected when creating service "ServiceB"')
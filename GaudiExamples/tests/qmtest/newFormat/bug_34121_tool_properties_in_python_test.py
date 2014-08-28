# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="../../scripts/bug_34121.py"

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        self.findReferenceBlock('''
ApplicationMgr       INFO Application Manager Initialized successfully
ApplicationMgr       INFO Application Manager Started successfully
Alg                  INFO executing....
Alg.Tool             INFO Double = 0
Alg                  INFO executing....
Alg.Tool             INFO Double = 1
Alg                  INFO executing....
Alg.Tool             INFO Double = 2
Alg                  INFO executing....
Alg.Tool             INFO Double = 3
Alg                  INFO executing....
Alg.Tool             INFO Double = 4
ApplicationMgr       INFO Application Manager Stopped successfully
Alg                  INFO finalizing....
''')
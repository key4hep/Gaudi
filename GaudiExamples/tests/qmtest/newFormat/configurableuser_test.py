# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="gaudirun.py"
        self.args=["-v","ConfigurableUser.py"]

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        self.countErrorLines()
        self.findReferenceBlock("ServiceManager      DEBUG Initializing service StatusCodeSvc")
        self.findReferenceBlock("""
==== Configuration completed ====
/***** Service ApplicationMgr/ApplicationMgr *******************************************************
""", id = "PostConf")
# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.args=["$GAUDIEXAMPLESROOT/options/ErrorLog.opts"]
        self.reference = "refs/ErrorLog.ref"
        self.error_reference="refs/ErrorLog_err.ref"

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        # Normalize the filename in the output of the test
        preproc = normalizeExamples + RegexpReplacer(r"(?<=\s)\S+ErrorLogTest\.cpp:", "ErrorLogTest.cpp:", "ErrorLogTest.cpp:")
        self.validateWithReference(preproc = preproc)
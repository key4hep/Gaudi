# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="../../scripts/Aida2RootEx.py"
        self.args=["-b"]
        self.reference = "refs/Aida2RootEx.ref"
        self.error_reference="refs/Aida2RootEx.err.ref"

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        preprocessor = (normalizeExamples + LineSkipper(regexps = [r'Info in <TCanvas::Print>: png file .* has been created']))
        self.validateWithReference(preproc = preprocessor)
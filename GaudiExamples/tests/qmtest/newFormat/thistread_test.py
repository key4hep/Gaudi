# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.args=["$GAUDIEXAMPLESROOT/options/THistRead.opts"]
        self.reference="refs/THistRead.ref"

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        preprocessor = normalizeExamples + \
            RegexpReplacer(when = "^THistSvc",
                orig = r"(stream: read[0-9]  name: tuple[0-9].rt  size: )([0-9]*)",
                repl = r"\1###")

        self.validateWithReference(preproc = preprocessor)
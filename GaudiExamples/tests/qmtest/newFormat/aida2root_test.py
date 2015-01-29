# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-3]
        self.args=["$GAUDIEXAMPLESROOT/options/Aida2Root.opts"]
        self.reference = "refs/Aida2Root.ref"
        self.options="""from Gaudi.Configuration import *
from Configurables import HelloWorld, AbortEventAlg

importOptions("Common.opts")

h1 = HelloWorld("HelloWorld1")
h2 = HelloWorld("HelloWorld2")

alg = AbortEventAlg(AbortedEventNumber = 3)

app = ApplicationMgr(TopAlg = [h1,alg,h2],
                     EvtSel = "NONE", EvtMax = 5)

MessageSvc().setDebug.append("EventLoopMgr") """

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        preprocessor = normalizeExamples + RegexpReplacer(when = "^Aida2Root",orig = r"(INFO.*'(skewness|kurtosis)(Err)?'.*)\|([0-9.e+\- ]*\|){2}",repl = r"\1| ### | ### |")
        self.validateWithReference(preproc = preprocessor)

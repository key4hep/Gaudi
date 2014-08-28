# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="gaudirun.py"
        self.args=["-v","-n","--option","MessageSvc().setFatal.append('AnotherAlg')"]
        self.options="""
from Gaudi.Configuration import *
MessageSvc().setDebug.append("AnAlgorithm")"""

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        conf_dict = eval('\n'.join(stdout.splitlines()))
        expected = {'MessageSvc': {'setDebug': ['AnAlgorithm'], 'setFatal': ['AnotherAlg']}}

        if conf_dict != expected:
            causes.append("configuration result")
            from pprint import PrettyPrinter
            pp = PrettyPrinter()
            result["GaudiTest.configuration.expected"] = result.Quote(pp.pformat(expected))
            result["GaudiTest.configuration.found"] = result.Quote(pp.pformat(conf_dict))
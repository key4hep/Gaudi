# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="gaudirun.py"
        self.args=["Timing.py"]

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        import re
        expected = ( r"Timing\s+SUCCESS\s+The timing is \(in us\)\s*\n" +
                     r"\|\s*\|\s*#\s*\|\s*Total\s*\|\s*Mean\+-RMS\s*\|\s*Min/Max\s*\|\s*\n" +
                     r"\|\s*\(1U\)\s*\|\s*\d*\s*\|(?:[-+.\deE /]+\|){3}\s*\n" +
                     r"\|\s*\(2U\)\s*\|\s*\d*\s*\|(?:[-+.\deE /]+\|){3}\s*\n" +
                     r"\|\s*\(3U\)\s*\|\s*\d*\s*\|(?:[-+.\deE /]+\|){3}\s*\n" )
        if not re.search(expected,stdout):
            causes.append("missing regexp match")
            result["GaudiTest.regexp"] = result.Quote(expected.replace("\\n","\n"))
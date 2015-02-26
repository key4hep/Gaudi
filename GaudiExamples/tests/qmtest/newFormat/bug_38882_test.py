# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="../../scripts/bug_38882.py"

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        expected = "=== bug_38882_test_alg Execute ==="
        if not expected in stdout:
            causes.append("missing signature")
            result["GaudiTest.expected_line"] = result.Quote(expected)
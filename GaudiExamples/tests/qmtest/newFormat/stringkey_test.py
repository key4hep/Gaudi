# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.program="../../scripts/StringKeyEx.py"

    def validator(self,stdout,stderr, result, causes, reference, error_reference):
        self.findReferenceBlock("""
StringKeys        SUCCESS check for       StringKey 'key'
StringKeys        SUCCESS  In Map 1: True
StringKeys        SUCCESS  In Map 2: True
StringKeys        SUCCESS  In Map 3: True
StringKeys        SUCCESS  In Map 4: True
StringKeys        SUCCESS  In Map01: True
StringKeys        SUCCESS  In Map02: True
StringKeys        SUCCESS  In Map03: True
StringKeys        SUCCESS  In Map04: True
StringKeys        SUCCESS check for std::string key 'rrr'
StringKeys        SUCCESS  In Map 1: False
StringKeys        SUCCESS  In Map 2: False
StringKeys        SUCCESS  In Map 3: False
StringKeys        SUCCESS  In Map 4: False
StringKeys        SUCCESS  In Map01: False
StringKeys        SUCCESS  In Map02: False
StringKeys        SUCCESS  In Map03: False
StringKeys        SUCCESS  In Map04: False
""", id = "block1")

        self.findReferenceBlock("""
StringKeys        SUCCESS check for       StringKey 'new Key'
StringKeys        SUCCESS  In Map 1: True
StringKeys        SUCCESS  In Map 2: True
StringKeys        SUCCESS  In Map 3: True
StringKeys        SUCCESS  In Map 4: True
StringKeys        SUCCESS  In Map01: True
StringKeys        SUCCESS  In Map02: True
StringKeys        SUCCESS  In Map03: True
StringKeys        SUCCESS  In Map04: True
StringKeys        SUCCESS check for std::string key 'rrr'
StringKeys        SUCCESS  In Map 1: True
StringKeys        SUCCESS  In Map 2: True
StringKeys        SUCCESS  In Map 3: True
StringKeys        SUCCESS  In Map 4: True
StringKeys        SUCCESS  In Map01: True
StringKeys        SUCCESS  In Map02: True
StringKeys        SUCCESS  In Map03: True
StringKeys        SUCCESS  In Map04: True
""", id = "block2")
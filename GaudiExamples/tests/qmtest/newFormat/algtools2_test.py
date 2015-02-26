# -*- coding: utf-8 -*-
import BaseTest
from BaseTest import *

class Test(BaseTest):

    def __init__(self):
        BaseTest.__init__(self)
        self.name = os.path.basename(__file__)[:-5]
        self.args=["$GAUDIEXAMPLESROOT/options/AlgTools2.opts"]
        self.reference = "refs/AlgTools2.ref"
#!/usr/bin/env python
#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################

import unittest
import sys


class TestCase(unittest.TestCase):
    def setUp(self):
        unittest.TestCase.setUp(self)

    def tearDown(self):
        unittest.TestCase.tearDown(self)

    def test_000_Import(self):
        try:
            import PyExample
        except ImportError:
            self.fail("Cannot import PyExample from %s" % sys.path)

    def test_010_Function(self):
        import PyExample
        self.assertEquals(PyExample.greet(), "hello, world")


if __name__ == '__main__':
    unittest.main(
        testRunner=unittest.TextTestRunner(stream=sys.stdout, verbosity=2))

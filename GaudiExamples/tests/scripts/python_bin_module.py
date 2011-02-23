#!/usr/bin/env python

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
    unittest.main(testRunner = unittest.TextTestRunner(stream = sys.stdout,
                                                       verbosity = 2))

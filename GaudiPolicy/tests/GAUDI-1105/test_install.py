#! /usr/bin/env python

## @Package test_install.py
#  @brief Unittests for install.py

import os
import os.path
import subprocess
import unittest
import uuid

import logging
msg = logging.getLogger(__name__)

# Unittests for this module
class installTests(unittest.TestCase):

    def setUp(self):
        self.fileToInstall = "testModule-" + str(uuid.uuid4()) + ".py"
        with open(self.fileToInstall, "w") as fh:
            fh.write('''#!/usr/bin/env python\npass\n''')

        self.targetDir = "testModuleDir-" + str(uuid.uuid4())

    def tearDown(self):
        for path in (os.path.join(self.targetDir, self.fileToInstall), self.fileToInstall, "install.log"):
            try:
                os.unlink(path)
            except OSError:
                pass
        try:
            os.rmdir(self.targetDir)
        except OSError:
            pass
    
    def test_installToExisitingPath(self):
        cmd = ["install.py", self.fileToInstall, self.targetDir]
        rc = subprocess.call(cmd)
        self.assertEqual(rc, 0)

if __name__ == '__main__':
    unittest.main()


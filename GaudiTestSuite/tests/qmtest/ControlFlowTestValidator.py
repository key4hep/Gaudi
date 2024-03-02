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
from GaudiTesting.BaseTest import FilePreprocessor, normalizeTestSuite


class DropUntil(FilePreprocessor):
    def __init__(self, regexp):
        import re

        self.regexp = re.compile(regexp)
        self._found = False

    def __call__(self, input):
        self._found = False
        return FilePreprocessor.__call__(self, input)

    def __processLine__(self, line):
        self._found = self._found or bool(self.regexp.search(line))
        if self._found:
            return line
        else:
            return None


class TakeUntil(FilePreprocessor):
    def __init__(self, regexp):
        import re

        self.regexp = re.compile(regexp)
        self._found = False

    def __call__(self, input):
        self._found = False
        return FilePreprocessor.__call__(self, input)

    def __processLine__(self, line):
        self._found = self._found or bool(self.regexp.search(line))
        if not self._found:
            return line
        else:
            return None


preprocessor = (
    normalizeTestSuite
    + DropUntil("ApplicationMgr       INFO Application Manager Started successfully")
    + TakeUntil("ToolSvc              INFO Removing all tools created by ToolSvc")
)

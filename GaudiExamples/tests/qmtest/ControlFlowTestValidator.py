from GaudiTesting.BaseTest import FilePreprocessor, normalizeExamples


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


preprocessor = (normalizeExamples +
                DropUntil('ApplicationMgr       INFO Application Manager Started successfully') +
                TakeUntil('ToolSvc              INFO Removing all tools created by ToolSvc'))

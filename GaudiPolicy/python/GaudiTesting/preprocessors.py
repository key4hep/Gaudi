#####################################################################################
# (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import re


class FilePreprocessor:
    """Base class for a callable that takes a file and returns a modified
    version of it."""

    def __processLine__(self, line):
        return line

    def __processFile__(self, lines):
        output = []
        for l in lines:
            l = self.__processLine__(l)
            if l:
                output.append(l)
        return output

    def __call__(self, input):
        if not isinstance(input, str):
            lines = input
            mergeback = False
        else:
            lines = input.splitlines()
            mergeback = True
        output = self.__processFile__(lines)
        if mergeback:
            output = "\n".join(output)
        return output

    def __add__(self, rhs):
        return FilePreprocessorSequence([self, rhs])


class FilePreprocessorSequence(FilePreprocessor):
    def __init__(self, members=[]):
        self.members = members

    def __add__(self, rhs):
        return FilePreprocessorSequence(self.members + [rhs])

    def __call__(self, input):
        output = input
        for pp in self.members:
            output = pp(output)
        return output


class LineSkipper(FilePreprocessor):
    def __init__(self, strings=[], regexps=[]):
        import re

        self.strings = strings
        self.regexps = list(map(re.compile, regexps))

    def __processLine__(self, line):
        for s in self.strings:
            if line.find(s) >= 0:
                return None
        for r in self.regexps:
            if r.search(line):
                return None
        return line


class BlockSkipper(FilePreprocessor):
    def __init__(self, start, end):
        self.start = start
        self.end = end
        self._skipping = False

    def __processLine__(self, line):
        if self.start in line:
            self._skipping = True
            return None
        elif self.end in line:
            self._skipping = False
        elif self._skipping:
            return None
        return line


class RegexpReplacer(FilePreprocessor):
    def __init__(self, orig, repl="", when=None):
        if when:
            when = re.compile(when)
        self._operations = [(when, re.compile(orig), repl)]

    def __add__(self, rhs):
        if isinstance(rhs, RegexpReplacer):
            res = RegexpReplacer("", "", None)
            res._operations = self._operations + rhs._operations
        else:
            res = FilePreprocessor.__add__(self, rhs)
        return res

    def __processLine__(self, line):
        for w, o, r in self._operations:
            if w is None or w.search(line):
                line = o.sub(r, line)
        return line


# Common preprocessors
maskPointers = RegexpReplacer("0x[0-9a-fA-F]{4,16}", "0x########")
normalizeDate = RegexpReplacer(
    "[0-2]?[0-9]:[0-5][0-9]:[0-5][0-9] [0-9]{4}[-/][01][0-9][-/][0-3][0-9][ A-Z]*",
    "00:00:00 1970-01-01",
)
normalizeEOL = FilePreprocessor()
normalizeEOL.__processLine__ = lambda line: str(line).rstrip() + "\n"

skipEmptyLines = FilePreprocessor()
# FIXME: that's ugly
skipEmptyLines.__processLine__ = lambda line: (line.strip() and line) or None

# Special preprocessor sorting the list of strings (whitespace separated)
#  that follow a signature on a single line


class LineSorter(FilePreprocessor):
    def __init__(self, signature):
        self.signature = signature
        self.siglen = len(signature)

    def __processLine__(self, line):
        pos = line.find(self.signature)
        if pos >= 0:
            line = line[: (pos + self.siglen)]
            lst = line[(pos + self.siglen) :].split()
            lst.sort()
            line += " ".join(lst)
        return line


class SortGroupOfLines(FilePreprocessor):
    """
    Sort group of lines matching a regular expression
    """

    def __init__(self, exp):
        self.exp = exp if hasattr(exp, "match") else re.compile(exp)

    def __processFile__(self, lines):
        match = self.exp.match
        output = []
        group = []
        for l in lines:
            if match(l):
                group.append(l)
            else:
                if group:
                    group.sort()
                    output.extend(group)
                    group = []
                output.append(l)
        return output


# Preprocessors for GaudiTestSuite
normalizeTestSuite = maskPointers + normalizeDate
for w, o, r in [
    ("TIMER", r"\s+[+-]?[0-9]+[0-9.e+-]*", " 0"),  # Normalize time output
    ("release all pending", r"^.*/([^/]*:.*)", r"\1"),
    ("^#.*file", r"file '.*[/\\]([^/\\]*)$", r"file '\1"),
    (
        "^JobOptionsSvc.*options successfully read in from",
        r"read in from .*[/\\]([^/\\]*)$",
        r"file \1",
    ),  # normalize path to options
    # Normalize UUID, except those ending with all 0s (i.e. the class IDs)
    (
        None,
        r"[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}(?!-0{12})-[0-9A-Fa-f]{12}",
        "00000000-0000-0000-0000-000000000000",
    ),
    # Absorb a change in ServiceLocatorHelper
    (
        "ServiceLocatorHelper::",
        "ServiceLocatorHelper::(create|locate)Service",
        "ServiceLocatorHelper::service",
    ),
    # Remove the leading 0 in Windows' exponential format
    (None, r"e([-+])0([0-9][0-9])", r"e\1\2"),
    # Output line changed in Gaudi v24
    (None, r"Service reference count check:", r"Looping over all active services..."),
    # Ignore count of declared properties (anyway they are all printed)
    (
        None,
        r"^(.*(DEBUG|SUCCESS) List of ALL properties of .*#properties = )\d+",
        r"\1NN",
    ),
    ("ApplicationMgr", r"(declareMultiSvcType|addMultiSvc): ", ""),
    (r"Property \['Name': Value\]", r"( =  '[^']+':)'(.*)'", r"\1\2"),
    ("TimelineSvc", "to file  'TimelineFile':", "to file "),
    ("DataObjectHandleBase", r'DataObjectHandleBase\("([^"]*)"\)', r"'\1'"),
]:
    normalizeTestSuite += RegexpReplacer(o, r, w)

lineSkipper = LineSkipper(
    [
        "//GP:",
        "JobOptionsSvc        INFO # ",
        "JobOptionsSvc     WARNING # ",
        "Time User",
        "Welcome to",
        "This machine has a speed",
        "running on",
        "ToolSvc.Sequenc...   INFO",
        "DataListenerSvc      INFO XML written to file:",
        "[INFO]",
        "[WARNING]",
        "DEBUG No writable file catalog found which contains FID:",
        "DEBUG Service base class initialized successfully",
        # changed between v20 and v21
        "DEBUG Incident  timing:",
        # introduced with patch #3487
        # changed the level of the message from INFO to
        # DEBUG
        "INFO  'CnvServices':[",
        # message removed because could be printed in constructor
        "DEBUG  'CnvServices':[",
        # The signal handler complains about SIGXCPU not
        # defined on some platforms
        "SIGXCPU",
        # Message removed with redesing of JobOptionsSvc
        "ServiceLocatorHelper::service: found service JobOptionsSvc",
        # Ignore warnings for properties case mismatch
        "mismatching case for property name:",
        # Message demoted to DEBUG in gaudi/Gaudi!992
        "Histograms saving not required.",
        # Message added in gaudi/Gaudi!577
        "Properties are dumped into",
        # Messages changed in gaudi/Gaudi!1426
        "WARNING no ROOT output file name",
        "INFO Writing ROOT histograms to:",
        "INFO Completed update of ROOT histograms in:",
        # absorb changes in data dependencies reports (https://gitlab.cern.ch/gaudi/Gaudi/-/merge_requests/1348)
        "Data Deps for ",
        "data dependencies:",
    ],
    regexps=[
        r"^JobOptionsSvc        INFO *$",
        r"^# ",  # Ignore python comments
        # skip the message reporting the version of the root file
        r"(Always|SUCCESS)\s*(Root f|[^ ]* F)ile version:",
        r"File '.*.xml' does not exist",
        r"INFO Refer to dataset .* by its file ID:",
        r"INFO Referring to dataset .* by its file ID:",
        r"INFO Disconnect from dataset",
        r"INFO Disconnected from dataset",
        r"INFO Disconnected data IO:",
        r"IncidentSvc\s*(DEBUG (Adding|Removing)|VERBOSE Calling)",
        # Ignore StatusCodeSvc related messages
        r".*StatusCodeSvc.*",
        r".*StatusCodeCheck.*",
        r"Num\s*\|\s*Function\s*\|\s*Source Library",
        r"^[-+]*\s*$",
        # Hide the fake error message coming from POOL/ROOT (ROOT 5.21)
        r"ERROR Failed to modify file: .* Errno=2 No such file or directory",
        # Hide unchecked StatusCodes from dictionaries
        r"^ +[0-9]+ \|.*ROOT",
        r"^ +[0-9]+ \|.*\|.*Dict",
        # Hide EventLoopMgr total timing report
        r"EventLoopMgr.*---> Loop Finished",
        r"HiveSlimEventLo.*---> Loop Finished",
        # Remove ROOT TTree summary table, which changes from one version to the
        # other
        r"^\*.*\*$",
        # Remove Histos Summaries
        r"SUCCESS\s*Booked \d+ Histogram\(s\)",
        r"^ \|",
        r"^ ID=",
        # Ignore added/removed properties
        r"Property(.*)'Audit(Algorithm|Tool|Service)s':",
        r"Property(.*)'Audit(Begin|End)Run':",
        # these were missing in tools
        r"Property(.*)'AuditRe(start|initialize)':",
        r"Property(.*)'Blocking':",
        # removed with gaudi/Gaudi!273
        r"Property(.*)'ErrorCount(er)?':",
        # added with gaudi/Gaudi!306
        r"Property(.*)'Sequential':",
        # added with gaudi/Gaudi!314
        r"Property(.*)'FilterCircularDependencies':",
        # removed with gaudi/Gaudi!316
        r"Property(.*)'IsClonable':",
        # ignore uninteresting/obsolete messages
        r"Property update for OutputLevel : new value =",
        r"EventLoopMgr\s*DEBUG Creating OutputStream",
    ],
)

normalizeTestSuite = (
    lineSkipper
    + normalizeTestSuite
    + skipEmptyLines
    + normalizeEOL
    + LineSorter("Services to release : ")
    + SortGroupOfLines(r"^\S+\s+(DEBUG|SUCCESS) Property \[\'Name\':")
)

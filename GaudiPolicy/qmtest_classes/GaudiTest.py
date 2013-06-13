########################################################################
# File:   GaudiTest.py
# Author: Marco Clemencic CERN/PH-LBC
########################################################################
__author__  = 'Marco Clemencic CERN/PH-LBC'
########################################################################
# Imports
########################################################################
import os
import sys
import re
import tempfile
import shutil
import string
import difflib
from subprocess import Popen, PIPE, STDOUT

import qm
from qm.test.classes.command import ExecTestBase
from qm.test.result_stream import ResultStream

### Needed by the re-implementation of TimeoutExecutable
import qm.executable
import time, signal
# The classes in this module are implemented differently depending on
# the operating system in use.
if sys.platform == "win32":
    import msvcrt
    import pywintypes
    from   threading import *
    import win32api
    import win32con
    import win32event
    import win32file
    import win32pipe
    import win32process
else:
    import cPickle
    import fcntl
    import select
    import qm.sigmask

########################################################################
# Utility Classes
########################################################################
class TemporaryEnvironment:
    """
    Class to changes the environment temporarily.
    """
    def __init__(self, orig = os.environ, keep_same = False):
        """
        Create a temporary environment on top of the one specified
        (it can be another TemporaryEnvironment instance).
        """
        #print "New environment"
        self.old_values = {}
        self.env = orig
        self._keep_same = keep_same

    def __setitem__(self,key,value):
        """
        Set an environment variable recording the previous value.
        """
        if key not in self.old_values :
            if key in self.env :
                if not self._keep_same or self.env[key] != value:
                    self.old_values[key] = self.env[key]
            else:
                self.old_values[key] = None
        self.env[key] = value

    def __getitem__(self,key):
        """
        Get an environment variable.
        Needed to provide the same interface as os.environ.
        """
        return self.env[key]

    def __delitem__(self,key):
        """
        Unset an environment variable.
        Needed to provide the same interface as os.environ.
        """
        if key not in self.env :
            raise KeyError(key)
        self.old_values[key] = self.env[key]
        del self.env[key]

    def keys(self):
        """
        Return the list of defined environment variables.
        Needed to provide the same interface as os.environ.
        """
        return self.env.keys()

    def items(self):
        """
        Return the list of (name,value) pairs for the defined environment variables.
        Needed to provide the same interface as os.environ.
        """
        return self.env.items()

    def __contains__(self,key):
        """
        Operator 'in'.
        Needed to provide the same interface as os.environ.
        """
        return key in self.env

    def restore(self):
        """
        Revert all the changes done to the orignal environment.
        """
        for key,value in self.old_values.items():
            if value is None:
                del self.env[key]
            else:
                self.env[key] = value
        self.old_values = {}

    def __del__(self):
        """
        Revert the changes on destruction.
        """
        #print "Restoring the environment"
        self.restore()

    def gen_script(self,shell_type):
        """
        Generate a shell script to reproduce the changes in the environment.
        """
        shells = [ 'csh', 'sh', 'bat' ]
        if shell_type not in shells:
            raise RuntimeError("Shell type '%s' unknown. Available: %s"%(shell_type,shells))
        out = ""
        for key,value in self.old_values.items():
            if key not in self.env:
                # unset variable
                if shell_type == 'csh':
                    out += 'unsetenv %s\n'%key
                elif shell_type == 'sh':
                    out += 'unset %s\n'%key
                elif shell_type == 'bat':
                    out += 'set %s=\n'%key
            else:
                # set variable
                if shell_type == 'csh':
                    out += 'setenv %s "%s"\n'%(key,self.env[key])
                elif shell_type == 'sh':
                    out += 'export %s="%s"\n'%(key,self.env[key])
                elif shell_type == 'bat':
                    out += 'set %s=%s\n'%(key,self.env[key])
        return out

class TempDir:
    """Small class for temporary directories.
    When instantiated, it creates a temporary directory and the instance
    behaves as the string containing the directory name.
    When the instance goes out of scope, it removes all the content of
    the temporary directory (automatic clean-up).
    """
    def __init__(self, keep = False, chdir = False):
        self.name = tempfile.mkdtemp()
        self._keep = keep
        self._origdir = None
        if chdir:
            self._origdir = os.getcwd()
            os.chdir(self.name)

    def __str__(self):
        return self.name

    def __del__(self):
        if self._origdir:
            os.chdir(self._origdir)
        if self.name and not self._keep:
            shutil.rmtree(self.name)

    def __getattr__(self,attr):
        return getattr(self.name,attr)

class TempFile:
    """Small class for temporary files.
    When instantiated, it creates a temporary directory and the instance
    behaves as the string containing the directory name.
    When the instance goes out of scope, it removes all the content of
    the temporary directory (automatic clean-up).
    """
    def __init__(self, suffix='', prefix='tmp', dir=None, text=False, keep = False):
        self.file = None
        self.name = None
        self._keep = keep

        self._fd, self.name = tempfile.mkstemp(suffix,prefix,dir,text)
        self.file = os.fdopen(self._fd,"r+")

    def __str__(self):
        return self.name

    def __del__(self):
        if self.file:
            self.file.close()
        if self.name and not self._keep:
            os.remove(self.name)

    def __getattr__(self,attr):
        return getattr(self.file,attr)

class CMT:
    """Small wrapper to call CMT.
    """
    def __init__(self,path=None):
        if path is None:
            path = os.getcwd()
        self.path = path

    def _run_cmt(self,command,args):
        # prepare command line
        if type(args) is str:
            args = [args]
        cmd = "cmt %s"%command
        for arg in args:
            cmd += ' "%s"'%arg

        # go to the execution directory
        olddir = os.getcwd()
        os.chdir(self.path)
        # run cmt
        result = os.popen4(cmd)[1].read()
        # return to the old directory
        os.chdir(olddir)
        return result

    def __getattr__(self,attr):
        return lambda args=[]: self._run_cmt(attr, args)

    def runtime_env(self,env = None):
        """Returns a dictionary containing the runtime environment produced by CMT.
        If a dictionary is passed a modified instance of it is returned.
        """
        if env is None:
            env = {}
        for l in self.setup("-csh").splitlines():
            l = l.strip()
            if l.startswith("setenv"):
                dummy,name,value = l.split(None,3)
                env[name] = value.strip('"')
            elif l.startswith("unsetenv"):
                dummy,name = l.split(None,2)
                if name in env:
                    del env[name]
        return env
    def show_macro(self,k):
        r = self.show(["macro",k])
        if r.find("CMT> Error: symbol not found") >= 0:
            return None
        else:
            return self.show(["macro_value",k]).strip()

## Locates an executable in the executables path ($PATH) and returns the full
#  path to it.
#  If the executable cannot be found, None is returned
def which(executable):
    """
    Locates an executable in the executables path ($PATH) and returns the full
    path to it.  An application is looked for with or without the '.exe' suffix.
    If the executable cannot be found, None is returned
    """
    if os.path.isabs(executable):
        if not os.path.exists(executable):
            if executable.endswith('.exe'):
                if os.path.exists(executable[:-4]):
                    return executable[:-4]
        return executable
    for d in os.environ.get("PATH").split(os.pathsep):
        fullpath = os.path.join(d, executable)
        if os.path.exists(fullpath):
            return fullpath
    if executable.endswith('.exe'):
        return which(executable[:-4])
    return None

def rationalizepath(p):
    p = os.path.normpath(os.path.expandvars(p))
    if os.path.exists(p):
        p = os.path.realpath(p)
    return p

########################################################################
# Output Validation Classes
########################################################################
class BasicOutputValidator:
    """Basic implementation of an option validator for Gaudi tests.
    This implementation is based on the standard (LCG) validation functions
    used in QMTest.
    """
    def __init__(self,ref,cause,result_key):
        self.reference = ref
        self.cause = cause
        self.result_key = result_key

    def __call__(self, out, result):
        """Validate the output of the program.

        'stdout' -- A string containing the data written to the standard output
        stream.

        'stderr' -- A string containing the data written to the standard error
        stream.

        'result' -- A 'Result' object. It may be used to annotate
        the outcome according to the content of stderr.

        returns -- A list of strings giving causes of failure."""

        causes = []
        # Check to see if theoutput matches.
        if not self.__CompareText(out, self.reference):
            causes.append(self.cause)
            result[self.result_key] = result.Quote(self.reference)

        return causes

    def __CompareText(self, s1, s2):
        """Compare 's1' and 's2', ignoring line endings.

        's1' -- A string.

        's2' -- A string.

        returns -- True if 's1' and 's2' are the same, ignoring
        differences in line endings."""

        # The "splitlines" method works independently of the line ending
        # convention in use.
        return s1.splitlines() == s2.splitlines()

class FilePreprocessor:
    """ Base class for a callable that takes a file and returns a modified
    version of it."""
    def __processLine__(self, line):
        return line
    def __call__(self, input):
        if hasattr(input,"__iter__"):
            lines = input
            mergeback = False
        else:
            lines = input.splitlines()
            mergeback = True
        output = []
        for l in lines:
            l = self.__processLine__(l)
            if l: output.append(l)
        if mergeback: output = '\n'.join(output)
        return output
    def __add__(self, rhs):
        return FilePreprocessorSequence([self,rhs])

class FilePreprocessorSequence(FilePreprocessor):
    def __init__(self, members = []):
        self.members = members
    def __add__(self, rhs):
        return FilePreprocessorSequence(self.members + [rhs])
    def __call__(self, input):
        output = input
        for pp in self.members:
            output = pp(output)
        return output

class LineSkipper(FilePreprocessor):
    def __init__(self, strings = [], regexps = []):
        import re
        self.strings = strings
        self.regexps = map(re.compile,regexps)

    def __processLine__(self, line):
        for s in self.strings:
            if line.find(s) >= 0: return None
        for r in self.regexps:
            if r.search(line): return None
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
    def __init__(self, orig, repl = "", when = None):
        if when:
            when = re.compile(when)
        self._operations = [ (when, re.compile(orig), repl) ]
    def __add__(self,rhs):
        if isinstance(rhs, RegexpReplacer):
            res = RegexpReplacer("","",None)
            res._operations = self._operations + rhs._operations
        else:
            res = FilePreprocessor.__add__(self, rhs)
        return res
    def __processLine__(self, line):
        for w,o,r in self._operations:
            if w is None or w.search(line):
                line = o.sub(r, line)
        return line

# Common preprocessors
maskPointers  = RegexpReplacer("0x[0-9a-fA-F]{4,16}","0x########")
normalizeDate = RegexpReplacer("[0-2]?[0-9]:[0-5][0-9]:[0-5][0-9] [0-9]{4}[-/][01][0-9][-/][0-3][0-9] *(CES?T)?",
                               "00:00:00 1970-01-01")
normalizeEOL = FilePreprocessor()
normalizeEOL.__processLine__ = lambda line: str(line).rstrip() + '\n'

skipEmptyLines = FilePreprocessor()
# FIXME: that's ugly
skipEmptyLines.__processLine__ = lambda line: (line.strip() and line) or None

## Special preprocessor sorting the list of strings (whitespace separated)
#  that follow a signature on a single line
class LineSorter(FilePreprocessor):
    def __init__(self, signature):
        self.signature = signature
        self.siglen = len(signature)
    def __processLine__(self, line):
        pos = line.find(self.signature)
        if pos >=0:
            line = line[:(pos+self.siglen)]
            lst = line[(pos+self.siglen):].split()
            lst.sort()
            line += " ".join(lst)
        return line

# Preprocessors for GaudiExamples
normalizeExamples = maskPointers + normalizeDate
for w,o,r in [
              #("TIMER.TIMER",r"[0-9]", "0"), # Normalize time output
              ("TIMER.TIMER",r"\s+[+-]?[0-9]+[0-9.]*", " 0"), # Normalize time output
              ("release all pending",r"^.*/([^/]*:.*)",r"\1"),
              ("0x########",r"\[.*/([^/]*.*)\]",r"[\1]"),
              ("^#.*file",r"file '.*[/\\]([^/\\]*)$",r"file '\1"),
              ("^JobOptionsSvc.*options successfully read in from",r"read in from .*[/\\]([^/\\]*)$",r"file \1"), # normalize path to options
              # Normalize UUID, except those ending with all 0s (i.e. the class IDs)
              (None,r"[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}(?!-0{12})-[0-9A-Fa-f]{12}","00000000-0000-0000-0000-000000000000"),
              # Absorb a change in ServiceLocatorHelper
              ("ServiceLocatorHelper::", "ServiceLocatorHelper::(create|locate)Service", "ServiceLocatorHelper::service"),
              # Remove the leading 0 in Windows' exponential format
              (None, r"e([-+])0([0-9][0-9])", r"e\1\2"),
              ]: #[ ("TIMER.TIMER","[0-9]+[0-9.]*", "") ]
    normalizeExamples += RegexpReplacer(o,r,w)
normalizeExamples = LineSkipper(["//GP:",
                                 "JobOptionsSvc        INFO # ",
                                 "JobOptionsSvc     WARNING # ",
                                 "Time User",
                                 "Welcome to",
                                 "This machine has a speed",
                                 "TIME:",
                                 "running on",
                                 "ToolSvc.Sequenc...   INFO",
                                 "DataListenerSvc      INFO XML written to file:",
                                 "[INFO]","[WARNING]",
                                 "DEBUG No writable file catalog found which contains FID:",
                                 "0 local", # hack for ErrorLogExample
                                 "DEBUG Service base class initialized successfully", # changed between v20 and v21
                                 "DEBUG Incident  timing:", # introduced with patch #3487
                                 "INFO  'CnvServices':[", # changed the level of the message from INFO to DEBUG
                                 # This comes from ROOT, when using GaudiPython
                                 'Note: (file "(tmpfile)", line 2) File "set" already loaded',
                                 # The signal handler complains about SIGXCPU not defined on some platforms
                                 'SIGXCPU',
                                 ],regexps = [
                                 r"^JobOptionsSvc        INFO *$",
                                 r"^#", # Ignore python comments
                                 r"(Always|SUCCESS)\s*(Root f|[^ ]* F)ile version:", # skip the message reporting the version of the root file
                                 r"0x[0-9a-fA-F#]+ *Algorithm::sysInitialize\(\) *\[", # hack for ErrorLogExample
                                 r"0x[0-9a-fA-F#]* *__gxx_personality_v0 *\[", # hack for ErrorLogExample
                                 r"File '.*.xml' does not exist",
                                 r"INFO Refer to dataset .* by its file ID:",
                                 r"INFO Referring to dataset .* by its file ID:",
                                 r"INFO Disconnect from dataset",
                                 r"INFO Disconnected from dataset",
                                 r"INFO Disconnected data IO:",
                                 r"IncidentSvc\s*(DEBUG (Adding|Removing)|VERBOSE Calling)",
                                 # I want to ignore the header of the unchecked StatusCode report
                                 r"^StatusCodeSvc.*listing all unchecked return codes:",
                                 r"^StatusCodeSvc\s*INFO\s*$",
                                 r"Num\s*\|\s*Function\s*\|\s*Source Library",
                                 r"^[-+]*\s*$",
                                 # Hide the fake error message coming from POOL/ROOT (ROOT 5.21)
                                 r"ERROR Failed to modify file: .* Errno=2 No such file or directory",
                                 # Hide unckeched StatusCodes  from dictionaries
                                 r"^ +[0-9]+ \|.*ROOT",
                                 r"^ +[0-9]+ \|.*\|.*Dict",
                                 # Remove ROOT TTree summary table, which changes from one version to the other
                                 r"^\*.*\*$",
                                 # Remove Histos Summaries
                                 r"SUCCESS\s*Booked \d+ Histogram\(s\)",
                                 r"^ \|",
                                 r"^ ID=",
                                 r'Note: .* File "vector" already loaded'
                                 ] ) + normalizeExamples + skipEmptyLines + \
                                  normalizeEOL + \
                                  LineSorter("Services to release : ")

class ReferenceFileValidator:
    def __init__(self, reffile, cause, result_key, preproc = normalizeExamples):
        self.reffile = os.path.expandvars(reffile)
        self.cause = cause
        self.result_key = result_key
        self.preproc = preproc
    def __call__(self, stdout, result):
        causes = []
        if os.path.isfile(self.reffile):
            orig = open(self.reffile).xreadlines()
            if self.preproc:
                orig = self.preproc(orig)
        else:
            orig = []

        new = stdout.splitlines()
        if self.preproc:
            new = self.preproc(new)
        #open(self.reffile + ".test","w").writelines(new)
        diffs = difflib.ndiff(orig,new,charjunk=difflib.IS_CHARACTER_JUNK)
        filterdiffs = map(lambda x: x.strip(),filter(lambda x: x[0] != " ",diffs))
        #filterdiffs = [x.strip() for x in diffs]
        if filterdiffs:
            result[self.result_key] = result.Quote("\n".join(filterdiffs))
            result[self.result_key] += result.Quote("""
Legend:
        -) reference file
        +) standard output of the test""")
            causes.append(self.cause)

        return causes

########################################################################
# Useful validation functions
########################################################################
def findReferenceBlock(reference, stdout, result, causes, signature_offset=0, signature=None,
                       id = None):
    """
    Given a block of text, tries to find it in the output.
    The block had to be identified by a signature line. By default, the first
    line is used as signature, or the line pointed to by signature_offset. If
    signature_offset points outside the block, a signature line can be passed as
    signature argument. Note: if 'signature' is None (the default), a negative
    signature_offset is interpreted as index in a list (e.g. -1 means the last
    line), otherwise the it is interpreted as the number of lines before the
    first one of the block the signature must appear.
    The parameter 'id' allow to distinguish between different calls to this
    function in the same validation code.
    """
    # split reference file, sanitize EOLs and remove empty lines
    reflines = filter(None,map(lambda s: s.rstrip(), reference.splitlines()))
    if not reflines:
        raise RuntimeError("Empty (or null) reference")
    # the same on standard output
    outlines = filter(None,map(lambda s: s.rstrip(), stdout.splitlines()))

    res_field = "GaudiTest.RefBlock"
    if id:
        res_field += "_%s" % id

    if signature is None:
        if signature_offset < 0:
            signature_offset = len(reference)+signature_offset
        signature = reflines[signature_offset]
    # find the reference block in the output file
    try:
        pos = outlines.index(signature)
        outlines = outlines[pos-signature_offset:pos+len(reflines)-signature_offset]
        if reflines != outlines:
            msg = "standard output"
            # I do not want 2 messages in causes if teh function is called twice
            if not msg in causes:
                causes.append(msg)
            result[res_field + ".observed"] = result.Quote("\n".join(outlines))
    except ValueError:
        causes.append("missing signature")
    result[res_field + ".signature"] = result.Quote(signature)
    if len(reflines) > 1 or signature != reflines[0]:
        result[res_field + ".expected"] = result.Quote("\n".join(reflines))

    return causes

def countErrorLines(expected = {'ERROR':0, 'FATAL':0}, **kwargs):
    """
    Count the number of messages with required severity (by default ERROR and FATAL)
    and check if their numbers match the expected ones (0 by default).
    The dictionary "expected" can be used to tune the number of errors and fatals
    allowed, or to limit the number of expected warnings etc.
    """
    stdout = kwargs["stdout"]
    result = kwargs["result"]
    causes = kwargs["causes"]

    # prepare the dictionary to record the extracted lines
    errors = {}
    for sev in expected:
        errors[sev] = []

    outlines = stdout.splitlines()
    from math import log10
    fmt = "%%%dd - %%s" % (int(log10(len(outlines))+1))

    linecount = 0
    for l in outlines:
        linecount += 1
        words = l.split()
        if len(words) >= 2 and words[1] in errors:
            errors[words[1]].append(fmt%(linecount,l.rstrip()))

    for e in errors:
        if len(errors[e]) != expected[e]:
            causes.append('%s(%d)'%(e,len(errors[e])))
            result["GaudiTest.lines.%s"%e] = result.Quote('\n'.join(errors[e]))
            result["GaudiTest.lines.%s.expected#"%e] = result.Quote(str(expected[e]))

    return causes


def _parseTTreeSummary(lines, pos):
    """
    Parse the TTree summary table in lines, starting from pos.
    Returns a tuple with the dictionary with the digested informations and the
    position of the first line after the summary.
    """
    result = {}
    i = pos + 1 # first line is a sequence of '*'
    count = len(lines)

    splitcols = lambda l: [ f.strip() for f in l.strip("*\n").split(':',2) ]
    def parseblock(ll):
        r = {}
        cols = splitcols(ll[0])
        r["Name"], r["Title"] = cols[1:]

        cols = splitcols(ll[1])
        r["Entries"] = int(cols[1])

        sizes = cols[2].split()
        r["Total size"] = int(sizes[2])
        if sizes[-1] == "memory":
            r["File size"] = 0
        else:
            r["File size"] = int(sizes[-1])

        cols = splitcols(ll[2])
        sizes = cols[2].split()
        if cols[0] == "Baskets":
            r["Baskets"] = int(cols[1])
            r["Basket size"] = int(sizes[2])
        r["Compression"] = float(sizes[-1])
        return r

    if i < (count - 3) and lines[i].startswith("*Tree"):
        result = parseblock(lines[i:i+3])
        result["Branches"] = {}
        i += 4
        while i < (count - 3) and lines[i].startswith("*Br"):
            if i < (count - 2) and lines[i].startswith("*Branch "):
                # skip branch header
                i += 3
                continue
            branch = parseblock(lines[i:i+3])
            result["Branches"][branch["Name"]] = branch
            i += 4

    return (result, i)

def findTTreeSummaries(stdout):
    """
    Scan stdout to find ROOT TTree summaries and digest them.
    """
    stars = re.compile(r"^\*+$")
    outlines = stdout.splitlines()
    nlines = len(outlines)
    trees = {}

    i = 0
    while i < nlines: #loop over the output
        # look for
        while i < nlines and not stars.match(outlines[i]):
            i += 1
        if i < nlines:
            tree, i = _parseTTreeSummary(outlines, i)
            if tree:
                trees[tree["Name"]] = tree

    return trees

def cmpTreesDicts(reference, to_check, ignore = None):
    """
    Check that all the keys in reference are in to_check too, with the same value.
    If the value is a dict, the function is called recursively. to_check can
    contain more keys than reference, that will not be tested.
    The function returns at the first difference found.
    """
    fail_keys = []
    # filter the keys in the reference dictionary
    if ignore:
        ignore_re = re.compile(ignore)
        keys = [ key for key in reference if not ignore_re.match(key) ]
    else:
        keys = reference.keys()
    # loop over the keys (not ignored) in the reference dictionary
    for k in keys:
        if k in to_check: # the key must be in the dictionary to_check
            if (type(reference[k]) is dict) and (type(to_check[k]) is dict):
                # if both reference and to_check values are dictionaries, recurse
                failed = fail_keys = cmpTreesDicts(reference[k], to_check[k], ignore)
            else:
                # compare the two values
                failed = to_check[k] != reference[k]
        else: # handle missing keys in the dictionary to check (i.e. failure)
            to_check[k] = None
            failed = True
        if failed:
            fail_keys.insert(0, k)
            break # exit from the loop at the first failure
    return fail_keys # return the list of keys bringing to the different values

def getCmpFailingValues(reference, to_check, fail_path):
    c = to_check
    r = reference
    for k in fail_path:
        c = c.get(k,None)
        r = r.get(k,None)
        if c is None or r is None:
            break # one of the dictionaries is not deep enough
    return (fail_path, r, c)

# signature of the print-out of the histograms
h_count_re = re.compile(r"^(.*)SUCCESS\s+Booked (\d+) Histogram\(s\) :\s+(.*)")

def parseHistosSummary(lines, pos):
    """
    Extract the histograms infos from the lines starting at pos.
    Returns the position of the first line after the summary block.
    """
    global h_count_re
    h_table_head = re.compile(r'SUCCESS\s+List of booked (1D|2D|3D|1D profile|2D profile) histograms in directory\s+"(\w*)"')
    h_short_summ = re.compile(r"ID=([^\"]+)\s+\"([^\"]+)\"\s+(.*)")

    nlines = len(lines)

    # decode header
    m = h_count_re.search(lines[pos])
    name = m.group(1).strip()
    total = int(m.group(2))
    header = {}
    for k, v in [ x.split("=") for x in  m.group(3).split() ]:
        header[k] = int(v)
    pos += 1
    header["Total"] = total

    summ = {}
    while pos < nlines:
        m = h_table_head.search(lines[pos])
        if m:
            t, d = m.groups(1) # type and directory
            t = t.replace(" profile", "Prof")
            pos += 1
            if pos < nlines:
                l = lines[pos]
            else:
                l = ""
            cont = {}
            if l.startswith(" | ID"):
                # table format
                titles = [ x.strip() for x in l.split("|")][1:]
                pos += 1
                while pos < nlines and lines[pos].startswith(" |"):
                    l = lines[pos]
                    values = [ x.strip() for x in l.split("|")][1:]
                    hcont = {}
                    for i in range(len(titles)):
                        hcont[titles[i]] = values[i]
                    cont[hcont["ID"]] = hcont
                    pos += 1
            elif l.startswith(" ID="):
                while pos < nlines and lines[pos].startswith(" ID="):
                    values = [ x.strip() for x in  h_short_summ.search(lines[pos]).groups() ]
                    cont[values[0]] = values
                    pos += 1
            else: # not interpreted
                raise RuntimeError("Cannot understand line %d: '%s'" % (pos, l))
            if not d in summ:
                summ[d] = {}
            summ[d][t] = cont
            summ[d]["header"] = header
        else:
            break
    if not summ:
        # If the full table is not present, we use only the header
        summ[name] = {"header": header}
    return summ, pos

def findHistosSummaries(stdout):
    """
    Scan stdout to find ROOT TTree summaries and digest them.
    """
    outlines = stdout.splitlines()
    nlines = len(outlines) - 1
    summaries = {}
    global h_count_re

    pos = 0
    while pos < nlines:
        summ = {}
        # find first line of block:
        match = h_count_re.search(outlines[pos])
        while pos < nlines and not match:
            pos += 1
            match = h_count_re.search(outlines[pos])
        if match:
            summ, pos = parseHistosSummary(outlines, pos)
        summaries.update(summ)
    return summaries

class GaudiFilterExecutable(qm.executable.Filter):
    def __init__(self, input, timeout = -1):
        """Create a new 'Filter'.

        'input' -- The string containing the input to provide to the
        child process.

        'timeout' -- As for 'TimeoutExecutable.__init__'."""

        super(GaudiFilterExecutable, self).__init__(input, timeout)
        self.__input = input
        self.__timeout = timeout
        self.stack_trace_file = None
        # Temporary file to pass the stack trace from one process to the other
        # The file must be closed and reopened when needed to avoid conflicts
        # between the processes
        tmpf = tempfile.mkstemp()
        os.close(tmpf[0])
        self.stack_trace_file = tmpf[1] # remember only the name

    def __UseSeparateProcessGroupForChild(self):
        """Copied from TimeoutExecutable to allow the re-implementation of
           _HandleChild.
        """
        if sys.platform == "win32":
            # In Windows 2000 (or later), we should use "jobs" by
            # analogy with UNIX process groups.  However, that
            # functionality is not (yet) provided by the Python Win32
            # extensions.
            return 0

        return self.__timeout >= 0 or self.__timeout == -2
    ##
    # Needs to replace the ones from RedirectedExecutable and TimeoutExecutable
    def _HandleChild(self):
        """Code copied from both FilterExecutable and TimeoutExecutable.
        """
        # Close the pipe ends that we do not need.
        if self._stdin_pipe:
            self._ClosePipeEnd(self._stdin_pipe[0])
        if self._stdout_pipe:
            self._ClosePipeEnd(self._stdout_pipe[1])
        if self._stderr_pipe:
            self._ClosePipeEnd(self._stderr_pipe[1])

        # The pipes created by 'RedirectedExecutable' must be closed
        # before the monitor process (created by 'TimeoutExecutable')
        # is created.  Otherwise, if the child process dies, 'select'
        # in the parent will not return if the monitor process may
        # still have one of the file descriptors open.

        super(qm.executable.TimeoutExecutable, self)._HandleChild()

        if self.__UseSeparateProcessGroupForChild():
            # Put the child into its own process group.  This step is
            # performed in both the parent and the child; therefore both
            # processes can safely assume that the creation of the process
            # group has taken place.
            child_pid = self._GetChildPID()
            try:
                os.setpgid(child_pid, child_pid)
            except:
                # The call to setpgid may fail if the child has exited,
                # or has already called 'exec'.  In that case, we are
                # guaranteed that the child has already put itself in the
                # desired process group.
                pass
            # Create the monitoring process.
            #
            # If the monitoring process is in parent's process group and
            # kills the child after waitpid has returned in the parent, we
            # may end up trying to kill a process group other than the one
            # that we intend to kill.  Therefore, we put the monitoring
            # process in the same process group as the child; that ensures
            # that the process group will persist until the monitoring
            # process kills it.
            self.__monitor_pid = os.fork()
            if self.__monitor_pid != 0:
                # Make sure that the monitoring process is placed into the
                # child's process group before the parent process calls
                # 'waitpid'.  In this way, we are guaranteed that the process
                # group as the child
                os.setpgid(self.__monitor_pid, child_pid)
            else:
                # Put the monitoring process into the child's process
                # group.  We know the process group still exists at
                # this point because either (a) we are in the process
                # group, or (b) the parent has not yet called waitpid.
                os.setpgid(0, child_pid)

                # Close all open file descriptors.  They are not needed
                # in the monitor process.  Furthermore, when the parent
                # closes the write end of the stdin pipe to the child,
                # we do not want the pipe to remain open; leaving the
                # pipe open in the monitor process might cause the child
                # to block waiting for additional input.
                try:
                    max_fds = os.sysconf("SC_OPEN_MAX")
                except:
                    max_fds = 256
                for fd in xrange(max_fds):
                    try:
                        os.close(fd)
                    except:
                        pass
                try:
                    if self.__timeout >= 0:
                        # Give the child time to run.
                        time.sleep (self.__timeout)
                        #######################################################
                        ### This is the interesting part: dump the stack trace to a file
                        if sys.platform == "linux2": # we should be have /proc and gdb
                            cmd = ["gdb",
                                   os.path.join("/proc", str(child_pid), "exe"),
                                   str(child_pid),
                                   "-batch", "-n", "-x",
                                   "'%s'" % os.path.join(os.path.dirname(__file__), "stack-trace.gdb")]
                            # FIXME: I wanted to use subprocess.Popen, but it doesn't want to work
                            #        in this context.
                            o = os.popen(" ".join(cmd)).read()
                            open(self.stack_trace_file,"w").write(o)
                        #######################################################

                        # Kill all processes in the child process group.
                        os.kill(0, signal.SIGKILL)
                    else:
                        # This call to select will never terminate.
                        select.select ([], [], [])
                finally:
                    # Exit.  This code is in a finally clause so that
                    # we are guaranteed to get here no matter what.
                    os._exit(0)
        elif self.__timeout >= 0 and sys.platform == "win32":
            # Create a monitoring thread.
            self.__monitor_thread = Thread(target = self.__Monitor)
            self.__monitor_thread.start()

    if sys.platform == "win32":

        def __Monitor(self):
            """Code copied from FilterExecutable.
            Kill the child if the timeout expires.

            This function is run in the monitoring thread."""

            # The timeout may be expressed as a floating-point value
            # on UNIX, but it must be an integer number of
            # milliseconds when passed to WaitForSingleObject.
            timeout = int(self.__timeout * 1000)
            # Wait for the child process to terminate or for the
            # timer to expire.
            result = win32event.WaitForSingleObject(self._GetChildPID(),
                                                    timeout)
            # If the timeout occurred, kill the child process.
            if result == win32con.WAIT_TIMEOUT:
                self.Kill()

########################################################################
# Test Classes
########################################################################
class GaudiExeTest(ExecTestBase):
    """Standard Gaudi test.
    """
    arguments = [
        qm.fields.TextField(
            name="program",
            title="Program",
            not_empty_text=1,
            description="""The path to the program.

            This field indicates the path to the program.  If it is not
            an absolute path, the value of the 'PATH' environment
            variable will be used to search for the program.
            If not specified, $GAUDIEXE or Gaudi.exe are used.
            """
            ),
        qm.fields.SetField(qm.fields.TextField(
            name="args",
            title="Argument List",
            description="""The command-line arguments.

            If this field is left blank, the program is run without any
            arguments.

            Use this field to specify the option files.

            An implicit 0th argument (the path to the program) is added
            automatically."""
            )),
        qm.fields.TextField(
            name="options",
            title="Options",
            description="""Options to be passed to the application.

            This field allows to pass a list of options to the main program
            without the need of a separate option file.

            The content of the field is written to a temporary file which name
            is passed the the application as last argument (appended to the
            field "Argument List".
            """,
            verbatim="true",
            multiline="true",
            default_value=""
            ),
        qm.fields.TextField(
            name="workdir",
            title="Working Directory",
            description="""Path to the working directory.

            If this field is left blank, the program will be run from the qmtest
            directory, otherwise from the directory specified.""",
            default_value=""
            ),
        qm.fields.TextField(
            name="reference",
            title="Reference Output",
            description="""Path to the file containing the reference output.

            If this field is left blank, any standard output will be considered
            valid.

            If the reference file is specified, any output on standard error is
            ignored."""
            ),
        qm.fields.TextField(
            name="error_reference",
            title="Reference for standard error",
            description="""Path to the file containing the reference for the standard error.

            If this field is left blank, any standard output will be considered
            valid.

            If the reference file is specified, any output on standard error is
            ignored."""
            ),
        qm.fields.SetField(qm.fields.TextField(
            name = "unsupported_platforms",
            title = "Unsupported Platforms",
            description = """Platform on which the test must not be run.

            List of regular expressions identifying the platforms on which the
            test is not run and the result is set to UNTESTED."""
            )),

        qm.fields.TextField(
            name = "validator",
            title = "Validator",
            description = """Function to validate the output of the test.

            If defined, the function is used to validate the products of the
            test.
            The function is called passing as arguments:
              self:   the test class instance
              stdout: the standard output of the executed test
              stderr: the standard error of the executed test
              result: the Result objects to fill with messages
            The function must return a list of causes for the failure.
            If specified, overrides standard output, standard error and
            reference files.
            """,
            verbatim="true",
            multiline="true",
            default_value=""
            ),

        qm.fields.BooleanField(
            name = "use_temp_dir",
            title = "Use temporary directory",
            description = """Use temporary directory.

            If set to true, use a temporary directory as working directory.
            """,
            default_value="false"
            ),

        qm.fields.IntegerField(
            name = "signal",
            title = "Expected signal",
            description = """Expect termination by signal.""",
            default_value=None
            ),
        ]

    def PlatformIsNotSupported(self, context, result):
        platform = self.GetPlatform()
        unsupported = [ re.compile(x)
                        for x in [ str(y).strip()
                                   for y in self.unsupported_platforms ]
                        if x
                       ]
        for p_re in unsupported:
            if p_re.search(platform):
                result.SetOutcome(result.UNTESTED)
                result[result.CAUSE] = 'Platform not supported.'
                return True
        return False

    def GetPlatform(self):
        """
        Return the platform Id defined in CMTCONFIG or SCRAM_ARCH.
        """
        arch = "None"
        # check architecture name
        if "CMTCONFIG" in os.environ:
            arch = os.environ["CMTCONFIG"]
        elif "SCRAM_ARCH" in os.environ:
            arch = os.environ["SCRAM_ARCH"]
        return arch

    def isWinPlatform(self):
        """
        Return True if the current platform is Windows.

        This function was needed because of the change in the CMTCONFIG format,
        from win32_vc71_dbg to i686-winxp-vc9-dbg.
        """
        platform = self.GetPlatform()
        return "winxp" in platform or platform.startswith("win")

    def _expandReferenceFileName(self, reffile):
        # if no file is passed, do nothing
        if not reffile:
            return ""

        # function to split an extension in constituents parts
        platformSplit = lambda p: set(p.split('-' in p and '-' or '_'))

        reference = os.path.normpath(os.path.expandvars(reffile))
        # old-style platform-specific reference name
        spec_ref = reference[:-3] + self.GetPlatform()[0:3] + reference[-3:]
        if os.path.isfile(spec_ref):
            reference = spec_ref
        else: # look for new-style platform specific reference files:
            # get all the files whose name start with the reference filename
            dirname, basename = os.path.split(reference)
            if not dirname: dirname = '.'
            head = basename + "."
            head_len = len(head)
            platform = platformSplit(self.GetPlatform())
            candidates = []
            for f in os.listdir(dirname):
                if f.startswith(head):
                    req_plat = platformSplit(f[head_len:])
                    if platform.issuperset(req_plat):
                        candidates.append( (len(req_plat), f) )
            if candidates: # take the one with highest matching
                # FIXME: it is not possible to say if x86_64-slc5-gcc43-dbg
                #        has to use ref.x86_64-gcc43 or ref.slc5-dbg
                candidates.sort()
                reference = os.path.join(dirname, candidates[-1][1])
        return reference

    def CheckTTreesSummaries(self, stdout, result, causes,
                             trees_dict = None,
                             ignore = r"Basket|.*size|Compression"):
        """
        Compare the TTree summaries in stdout with the ones in trees_dict or in
        the reference file. By default ignore the size, compression and basket
        fields.
        The presence of TTree summaries when none is expected is not a failure.
        """
        if trees_dict is None:
            reference = self._expandReferenceFileName(self.reference)
            # call the validator if the file exists
            if reference and os.path.isfile(reference):
                trees_dict = findTTreeSummaries(open(reference).read())
            else:
                trees_dict = {}

        from pprint import PrettyPrinter
        pp = PrettyPrinter()
        if trees_dict:
            result["GaudiTest.TTrees.expected"] = result.Quote(pp.pformat(trees_dict))
            if ignore:
                result["GaudiTest.TTrees.ignore"] = result.Quote(ignore)

        trees = findTTreeSummaries(stdout)
        failed = cmpTreesDicts(trees_dict, trees, ignore)
        if failed:
            causes.append("trees summaries")
            msg = "%s: %s != %s" % getCmpFailingValues(trees_dict, trees, failed)
            result["GaudiTest.TTrees.failure_on"] = result.Quote(msg)
            result["GaudiTest.TTrees.found"] = result.Quote(pp.pformat(trees))

        return causes

    def CheckHistosSummaries(self, stdout, result, causes,
                             dict = None,
                             ignore = None):
        """
        Compare the TTree summaries in stdout with the ones in trees_dict or in
        the reference file. By default ignore the size, compression and basket
        fields.
        The presence of TTree summaries when none is expected is not a failure.
        """
        if dict is None:
            reference = self._expandReferenceFileName(self.reference)
            # call the validator if the file exists
            if reference and os.path.isfile(reference):
                dict = findHistosSummaries(open(reference).read())
            else:
                dict = {}

        from pprint import PrettyPrinter
        pp = PrettyPrinter()
        if dict:
            result["GaudiTest.Histos.expected"] = result.Quote(pp.pformat(dict))
            if ignore:
                result["GaudiTest.Histos.ignore"] = result.Quote(ignore)

        histos = findHistosSummaries(stdout)
        failed = cmpTreesDicts(dict, histos, ignore)
        if failed:
            causes.append("histos summaries")
            msg = "%s: %s != %s" % getCmpFailingValues(dict, histos, failed)
            result["GaudiTest.Histos.failure_on"] = result.Quote(msg)
            result["GaudiTest.Histos.found"] = result.Quote(pp.pformat(histos))

        return causes

    def ValidateWithReference(self, stdout, stderr, result, causes, preproc = None):
        """
        Default validation action: compare standard output and error to the
        reference files.
        """
        # set the default output preprocessor
        if preproc is None:
            preproc = normalizeExamples
        # check standard output
        reference = self._expandReferenceFileName(self.reference)
        # call the validator if the file exists
        if reference and os.path.isfile(reference):
            result["GaudiTest.output_reference"] = reference
            causes += ReferenceFileValidator(reference,
                                             "standard output",
                                             "GaudiTest.output_diff",
                                             preproc = preproc)(stdout, result)

        # Compare TTree summaries
        causes = self.CheckTTreesSummaries(stdout, result, causes)
        causes = self.CheckHistosSummaries(stdout, result, causes)

        if causes: # Write a new reference file for stdout
            try:
                newref = open(reference + ".new","w")
                # sanitize newlines
                for l in stdout.splitlines():
                    newref.write(l.rstrip() + '\n')
                del newref # flush and close
            except IOError:
                # Ignore IO errors when trying to update reference files
                # because we may be in a read-only filesystem
                pass

        # check standard error
        reference = self._expandReferenceFileName(self.error_reference)
        # call the validator if we have a file to use
        if reference and os.path.isfile(reference):
            result["GaudiTest.error_reference"] = reference
            newcauses = ReferenceFileValidator(reference,
                                               "standard error",
                                               "GaudiTest.error_diff",
                                               preproc = preproc)(stderr, result)
            causes += newcauses
            if newcauses: # Write a new reference file for stdedd
                newref = open(reference + ".new","w")
                # sanitize newlines
                for l in stderr.splitlines():
                    newref.write(l.rstrip() + '\n')
                del newref # flush and close
        else:
            causes += BasicOutputValidator(self.stderr,
                                           "standard error",
                                           "ExecTest.expected_stderr")(stderr, result)

        return causes

    def ValidateOutput(self, stdout, stderr, result):
        causes = []
        # if the test definition contains a custom validator, use it
        if self.validator.strip() != "":
            class CallWrapper(object):
                """
                Small wrapper class to dynamically bind some default arguments
                to a callable.
                """
                def __init__(self, callable, extra_args = {}):
                    self.callable = callable
                    self.extra_args = extra_args
                    # get the list of names of positional arguments
                    from inspect import getargspec
                    self.args_order = getargspec(callable)[0]
                    # Remove "self" from the list of positional arguments
                    # since it is added automatically
                    if self.args_order[0] == "self":
                        del self.args_order[0]
                def __call__(self, *args, **kwargs):
                    # Check which positional arguments are used
                    positional = self.args_order[:len(args)]

                    kwargs = dict(kwargs) # copy the arguments dictionary
                    for a in self.extra_args:
                        # use "extra_args" for the arguments not specified as
                        # positional or keyword
                        if a not in positional and a not in kwargs:
                            kwargs[a] = self.extra_args[a]
                    return apply(self.callable, args, kwargs)
            # local names to be exposed in the script
            exported_symbols = {"self":self,
                                "stdout":stdout,
                                "stderr":stderr,
                                "result":result,
                                "causes":causes,
                                "findReferenceBlock":
                                    CallWrapper(findReferenceBlock, {"stdout":stdout,
                                                                     "result":result,
                                                                     "causes":causes}),
                                "validateWithReference":
                                    CallWrapper(self.ValidateWithReference, {"stdout":stdout,
                                                                             "stderr":stderr,
                                                                             "result":result,
                                                                             "causes":causes}),
                                "countErrorLines":
                                    CallWrapper(countErrorLines, {"stdout":stdout,
                                                                  "result":result,
                                                                  "causes":causes}),
                                "checkTTreesSummaries":
                                    CallWrapper(self.CheckTTreesSummaries, {"stdout":stdout,
                                                                            "result":result,
                                                                            "causes":causes}),
                                "checkHistosSummaries":
                                    CallWrapper(self.CheckHistosSummaries, {"stdout":stdout,
                                                                            "result":result,
                                                                            "causes":causes}),

                                }
            exec self.validator in globals(), exported_symbols
        else:
            self.ValidateWithReference(stdout, stderr, result, causes)

        return causes

    def DumpEnvironment(self, result):
        """
        Add the content of the environment to the result object.

        Copied from the QMTest class of COOL.
        """
        vars = os.environ.keys()
        vars.sort()
        result['GaudiTest.environment'] = \
            result.Quote('\n'.join(["%s=%s"%(v,os.environ[v]) for v in vars]))

    def Run(self, context, result):
        """Run the test.

        'context' -- A 'Context' giving run-time parameters to the
        test.

        'result' -- A 'Result' object.  The outcome will be
        'Result.PASS' when this method is called.  The 'result' may be
        modified by this method to indicate outcomes other than
        'Result.PASS' or to add annotations."""

        # Check if the platform is supported
        if self.PlatformIsNotSupported(context, result):
            return

        # Prepare program name and arguments (expanding variables, and converting to absolute)
        if self.program:
            prog = rationalizepath(self.program)
        elif "GAUDIEXE" in os.environ:
            prog = os.environ["GAUDIEXE"]
        else:
            prog = "Gaudi.exe"
        self.program = prog

        dummy, prog_ext = os.path.splitext(prog)
        if prog_ext not in [ ".exe", ".py", ".bat" ] and self.isWinPlatform():
            prog += ".exe"
            prog_ext = ".exe"

        prog = which(prog) or prog

        # Convert paths to absolute paths in arguments and reference files
        args = map(rationalizepath, self.args)
        self.reference = rationalizepath(self.reference)
        self.error_reference = rationalizepath(self.error_reference)


        # check if the user provided inline options
        tmpfile = None
        if self.options.strip():
            ext = ".opts"
            if re.search(r"from\s*Gaudi.Configuration\s*import\s*\*", self.options):
                ext = ".py"
            tmpfile = TempFile(ext)
            tmpfile.writelines("\n".join(self.options.splitlines()))
            tmpfile.flush()
            args.append(tmpfile.name)
            result["GaudiTest.options"] = result.Quote(self.options)

        # if the program is a python file, execute it through python
        if prog_ext == ".py":
            args.insert(0,prog)
            if self.isWinPlatform():
                prog = which("python.exe") or "python.exe"
            else:
                prog = which("python") or "python"

        # Change to the working directory if specified or to the default temporary
        origdir = os.getcwd()
        if self.workdir:
            os.chdir(str(os.path.normpath(os.path.expandvars(self.workdir))))
        elif self.use_temp_dir == "true":
            if "QMTEST_TMPDIR" in os.environ:
                qmtest_tmpdir = os.environ["QMTEST_TMPDIR"]
                if not os.path.exists(qmtest_tmpdir):
                    os.makedirs(qmtest_tmpdir)
                os.chdir(qmtest_tmpdir)
            elif "qmtest.tmpdir" in context:
                os.chdir(context["qmtest.tmpdir"])

        if "QMTEST_IGNORE_TIMEOUT" not in os.environ:
            self.timeout = max(self.timeout,600)
        else:
            self.timeout = -1

        try:
            # Generate eclipse.org debug launcher for the test
            self._CreateEclipseLaunch(prog, args, destdir = os.path.join(origdir, '.eclipse'))
            # Run the test
            self.RunProgram(prog,
                            [ prog ] + args,
                            context, result)
            # Record the content of the enfironment for failing tests
            if result.GetOutcome() not in [ result.PASS ]:
                self.DumpEnvironment(result)
        finally:
            # revert to the original directory
            os.chdir(origdir)

    def RunProgram(self, program, arguments, context, result):
        """Run the 'program'.

        'program' -- The path to the program to run.

        'arguments' -- A list of the arguments to the program.  This
        list must contain a first argument corresponding to 'argv[0]'.

        'context' -- A 'Context' giving run-time parameters to the
        test.

        'result' -- A 'Result' object.  The outcome will be
        'Result.PASS' when this method is called.  The 'result' may be
        modified by this method to indicate outcomes other than
        'Result.PASS' or to add annotations.

        @attention: This method has been copied from command.ExecTestBase
                    (QMTest 2.3.0) and modified to keep stdout and stderr
                    for tests that have been terminated by a signal.
                    (Fundamental for debugging in the Application Area)
        """

        # Construct the environment.
        environment = self.MakeEnvironment(context)
        # FIXME: whithout this, we get some spurious '\x1b[?1034' in the std out on SLC6
        if "slc6" in environment.get('CMTCONFIG', ''):
            environment['TERM'] = 'dumb'
        # Create the executable.
        if self.timeout >= 0:
            timeout = self.timeout
        else:
            # If no timeout was specified, we sill run this process in a
            # separate process group and kill the entire process group
            # when the child is done executing.  That means that
            # orphaned child processes created by the test will be
            # cleaned up.
            timeout = -2
        e = GaudiFilterExecutable(self.stdin, timeout)
        # Run it.
        exit_status = e.Run(arguments, environment, path = program)
        # Get the stack trace from the temporary file (if present)
        if e.stack_trace_file and os.path.exists(e.stack_trace_file):
            stack_trace = open(e.stack_trace_file).read()
            os.remove(e.stack_trace_file)
        else:
            stack_trace = None
        if stack_trace:
            result["ExecTest.stack_trace"] = result.Quote(stack_trace)

        # If the process terminated normally, check the outputs.
        if (sys.platform == "win32" or os.WIFEXITED(exit_status)
            or self.signal == os.WTERMSIG(exit_status)):
            # There are no causes of failure yet.
            causes = []
            # The target program terminated normally.  Extract the
            # exit code, if this test checks it.
            if self.exit_code is None:
                exit_code = None
            elif sys.platform == "win32":
                exit_code = exit_status
            else:
                exit_code = os.WEXITSTATUS(exit_status)
            # Get the output generated by the program.
            stdout = e.stdout
            stderr = e.stderr
            # Record the results.
            result["ExecTest.exit_code"] = str(exit_code)
            result["ExecTest.stdout"] = result.Quote(stdout)
            result["ExecTest.stderr"] = result.Quote(stderr)
            # Check to see if the exit code matches.
            if exit_code != self.exit_code:
                causes.append("exit_code")
                result["ExecTest.expected_exit_code"] \
                    = str(self.exit_code)
            # Validate the output.
            causes += self.ValidateOutput(stdout, stderr, result)
            # If anything went wrong, the test failed.
            if causes:
                result.Fail("Unexpected %s." % string.join(causes, ", "))
        elif os.WIFSIGNALED(exit_status):
            # The target program terminated with a signal.  Construe
            # that as a test failure.
            signal_number = str(os.WTERMSIG(exit_status))
            if not stack_trace:
                result.Fail("Program terminated by signal.")
            else:
                # The presence of stack_trace means tha we stopped the job because
                # of a time-out
                result.Fail("Exceeded time limit (%ds), terminated." % timeout)
            result["ExecTest.signal_number"] = signal_number
            result["ExecTest.stdout"] = result.Quote(e.stdout)
            result["ExecTest.stderr"] = result.Quote(e.stderr)
            if self.signal:
                result["ExecTest.expected_signal_number"] = str(self.signal)
        elif os.WIFSTOPPED(exit_status):
            # The target program was stopped.  Construe that as a
            # test failure.
            signal_number = str(os.WSTOPSIG(exit_status))
            if not stack_trace:
                result.Fail("Program stopped by signal.")
            else:
                # The presence of stack_trace means tha we stopped the job because
                # of a time-out
                result.Fail("Exceeded time limit (%ds), stopped." % timeout)
            result["ExecTest.signal_number"] = signal_number
            result["ExecTest.stdout"] = result.Quote(e.stdout)
            result["ExecTest.stderr"] = result.Quote(e.stderr)
        else:
            # The target program terminated abnormally in some other
            # manner.  (This shouldn't normally happen...)
            result.Fail("Program did not terminate normally.")

        # Marco Cl.: This is a special trick to fix a "problem" with the output
        # of gaudi jobs when they use colors
        esc = '\x1b'
        repr_esc = '\\x1b'
        result["ExecTest.stdout"] = result["ExecTest.stdout"].replace(esc,repr_esc)
        # TODO: (MCl) improve the hack for colors in standard output
        #             may be converting them to HTML tags

    def _CreateEclipseLaunch(self, prog, args, destdir = None):
        # Find the project name used in ecplise.
        # The name is in a file called ".project" in one of the parent directories
        projbasedir = os.path.normpath(destdir)
        while not os.path.exists(os.path.join(projbasedir, ".project")):
            oldprojdir = projbasedir
            projbasedir = os.path.normpath(os.path.join(projbasedir, os.pardir))
            # FIXME: the root level is invariant when trying to go up one level,
            #        but it must be cheched on windows
            if oldprojdir == projbasedir:
                # If we cannot find a .project, so no point in creating a .launch file
                return
        # Ensure that we have a place where to write.
        if not os.path.exists(destdir):
            os.makedirs(destdir)
        # Use ElementTree to parse the XML file
        from xml.etree import ElementTree as ET
        t = ET.parse(os.path.join(projbasedir, ".project"))
        projectName = t.find("name").text

        # prepare the name/path of the generated file
        destfile = "%s.launch" % self._Runnable__id
        if destdir:
            destfile = os.path.join(destdir, destfile)

        if self.options.strip():
            # this means we have some custom options in the qmt file, so we have
            # to copy them from the temporary file at the end of the arguments
            # in another file
            tempfile = args.pop()
            optsfile = destfile + os.path.splitext(tempfile)[1]
            shutil.copyfile(tempfile, optsfile)
            args.append(optsfile)

        # prepare the data to insert in the XML file
        from xml.sax.saxutils import quoteattr # useful to quote XML special chars
        data = {}
        # Note: the "quoteattr(k)" is not needed because special chars cannot be part of a variable name,
        # but it doesn't harm.
        data["environment"] = "\n".join(['<mapEntry key=%s value=%s/>' % (quoteattr(k), quoteattr(v))
                                         for k, v in os.environ.iteritems()
                                         if k not in ('MAKEOVERRIDES', 'MAKEFLAGS', 'MAKELEVEL')])

        data["exec"] = which(prog) or prog
        if os.path.basename(data["exec"]).lower().startswith("python"):
            data["stopAtMain"] = "false" # do not stop at main when debugging Python scripts
        else:
            data["stopAtMain"] = "true"

        data["args"] = "&#10;".join(map(rationalizepath, args))
        if self.isWinPlatform():
            data["args"] = "&#10;".join(["/debugexe"] + map(rationalizepath, [data["exec"]] + args))
            data["exec"] = which("vcexpress.exe")

        if not self.use_temp_dir:
            data["workdir"] = os.getcwd()
        else:
            # If the test is using a tmporary directory, it is better to run it
            # in the same directory as the .launch file when debugged in eclipse
            data["workdir"] = destdir

        data["project"] = projectName.strip()

        # Template for the XML file, based on eclipse 3.4
        xml = """<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<launchConfiguration type="org.eclipse.cdt.launch.applicationLaunchType">
<booleanAttribute key="org.eclipse.cdt.debug.mi.core.AUTO_SOLIB" value="true"/>
<listAttribute key="org.eclipse.cdt.debug.mi.core.AUTO_SOLIB_LIST"/>
<stringAttribute key="org.eclipse.cdt.debug.mi.core.DEBUG_NAME" value="gdb"/>
<stringAttribute key="org.eclipse.cdt.debug.mi.core.GDB_INIT" value=".gdbinit"/>
<listAttribute key="org.eclipse.cdt.debug.mi.core.SOLIB_PATH"/>
<booleanAttribute key="org.eclipse.cdt.debug.mi.core.STOP_ON_SOLIB_EVENTS" value="false"/>
<booleanAttribute key="org.eclipse.cdt.debug.mi.core.breakpointsFullPath" value="false"/>
<stringAttribute key="org.eclipse.cdt.debug.mi.core.commandFactory" value="org.eclipse.cdt.debug.mi.core.standardCommandFactory"/>
<stringAttribute key="org.eclipse.cdt.debug.mi.core.protocol" value="mi"/>
<booleanAttribute key="org.eclipse.cdt.debug.mi.core.verboseMode" value="false"/>
<intAttribute key="org.eclipse.cdt.launch.ATTR_BUILD_BEFORE_LAUNCH_ATTR" value="0"/>
<stringAttribute key="org.eclipse.cdt.launch.COREFILE_PATH" value=""/>
<stringAttribute key="org.eclipse.cdt.launch.DEBUGGER_ID" value="org.eclipse.cdt.debug.mi.core.CDebuggerNew"/>
<stringAttribute key="org.eclipse.cdt.launch.DEBUGGER_REGISTER_GROUPS" value=""/>
<stringAttribute key="org.eclipse.cdt.launch.DEBUGGER_START_MODE" value="run"/>
<booleanAttribute key="org.eclipse.cdt.launch.DEBUGGER_STOP_AT_MAIN" value="%(stopAtMain)s"/>
<stringAttribute key="org.eclipse.cdt.launch.DEBUGGER_STOP_AT_MAIN_SYMBOL" value="main"/>
<booleanAttribute key="org.eclipse.cdt.launch.ENABLE_REGISTER_BOOKKEEPING" value="false"/>
<booleanAttribute key="org.eclipse.cdt.launch.ENABLE_VARIABLE_BOOKKEEPING" value="false"/>
<stringAttribute key="org.eclipse.cdt.launch.FORMAT" value="&lt;?xml version=&quot;1.0&quot; encoding=&quot;UTF-8&quot; standalone=&quot;no&quot;?&gt;&lt;contentList/&gt;"/>
<stringAttribute key="org.eclipse.cdt.launch.GLOBAL_VARIABLES" value="&lt;?xml version=&quot;1.0&quot; encoding=&quot;UTF-8&quot; standalone=&quot;no&quot;?&gt;&#10;&lt;globalVariableList/&gt;&#10;"/>
<stringAttribute key="org.eclipse.cdt.launch.MEMORY_BLOCKS" value="&lt;?xml version=&quot;1.0&quot; encoding=&quot;UTF-8&quot; standalone=&quot;no&quot;?&gt;&#10;&lt;memoryBlockExpressionList/&gt;&#10;"/>
<stringAttribute key="org.eclipse.cdt.launch.PROGRAM_ARGUMENTS" value="%(args)s"/>
<stringAttribute key="org.eclipse.cdt.launch.PROGRAM_NAME" value="%(exec)s"/>
<stringAttribute key="org.eclipse.cdt.launch.PROJECT_ATTR" value="%(project)s"/>
<stringAttribute key="org.eclipse.cdt.launch.PROJECT_BUILD_CONFIG_ID_ATTR" value=""/>
<stringAttribute key="org.eclipse.cdt.launch.WORKING_DIRECTORY" value="%(workdir)s"/>
<booleanAttribute key="org.eclipse.cdt.launch.ui.ApplicationCDebuggerTab.DEFAULTS_SET" value="true"/>
<booleanAttribute key="org.eclipse.cdt.launch.use_terminal" value="true"/>
<listAttribute key="org.eclipse.debug.core.MAPPED_RESOURCE_PATHS">
<listEntry value="/%(project)s"/>
</listAttribute>
<listAttribute key="org.eclipse.debug.core.MAPPED_RESOURCE_TYPES">
<listEntry value="4"/>
</listAttribute>
<booleanAttribute key="org.eclipse.debug.core.appendEnvironmentVariables" value="false"/>
<mapAttribute key="org.eclipse.debug.core.environmentVariables">
%(environment)s
</mapAttribute>
<mapAttribute key="org.eclipse.debug.core.preferred_launchers">
<mapEntry key="[debug]" value="org.eclipse.cdt.cdi.launch.localCLaunch"/>
</mapAttribute>
<listAttribute key="org.eclipse.debug.ui.favoriteGroups">
<listEntry value="org.eclipse.debug.ui.launchGroup.debug"/>
</listAttribute>
</launchConfiguration>
""" % data

        # Write the output file
        open(destfile, "w").write(xml)
        #open(destfile + "_copy.xml", "w").write(xml)


try:
    import json
except ImportError:
    # Use simplejson for LCG
    import simplejson as json

class HTMLResultStream(ResultStream):
    """An 'HTMLResultStream' writes its output to a set of HTML files.

    The argument 'dir' is used to select the destination directory for the HTML
    report.
    The destination directory may already contain the report from a previous run
    (for example of a different package), in which case it will be extended to
    include the new data.
    """
    arguments = [
        qm.fields.TextField(
            name = "dir",
            title = "Destination Directory",
            description = """The name of the directory.

            All results will be written to the directory indicated.""",
            verbatim = "true",
            default_value = ""),
    ]

    def __init__(self, arguments = None, **args):
        """Prepare the destination directory.

        Creates the destination directory and store in it some preliminary
        annotations and the static files found in the template directory
        'html_report'.
        """
        ResultStream.__init__(self, arguments, **args)
        self._summary = []
        self._summaryFile = os.path.join(self.dir, "summary.json")
        self._annotationsFile = os.path.join(self.dir, "annotations.json")
        # Prepare the destination directory using the template
        templateDir = os.path.join(os.path.dirname(__file__), "html_report")
        if not os.path.isdir(self.dir):
            os.makedirs(self.dir)
        # Copy the files in the template directory excluding the directories
        for f in os.listdir(templateDir):
            src = os.path.join(templateDir, f)
            dst = os.path.join(self.dir, f)
            if not os.path.isdir(src) and not os.path.exists(dst):
                shutil.copy(src, dst)
        # Add some non-QMTest attributes
        if "CMTCONFIG" in os.environ:
            self.WriteAnnotation("cmt.cmtconfig", os.environ["CMTCONFIG"])
        import socket
        self.WriteAnnotation("hostname", socket.gethostname())

    def _updateSummary(self):
        """Helper function to extend the global summary file in the destination
        directory.
        """
        if os.path.exists(self._summaryFile):
            oldSummary = json.load(open(self._summaryFile))
        else:
            oldSummary = []
        ids = set([ i["id"] for i in self._summary ])
        newSummary = [ i for i in oldSummary if i["id"] not in ids ]
        newSummary.extend(self._summary)
        json.dump(newSummary, open(self._summaryFile, "w"),
                  sort_keys = True)

    def WriteAnnotation(self, key, value):
        """Writes the annotation to the annotation file.
        If the key is already present with a different value, the value becomes
        a list and the new value is appended to it, except for start_time and
        end_time.
        """
        # Initialize the annotation dict from the file (if present)
        if os.path.exists(self._annotationsFile):
            annotations = json.load(open(self._annotationsFile))
        else:
            annotations = {}
        # hack because we do not have proper JSON support
        key, value = map(str, [key, value])
        if key == "qmtest.run.start_time":
            # Special handling of the start time:
            # if we are updating a result, we have to keep the original start
            # time, but remove the original end time to mark the report to be
            # in progress.
            if key not in annotations:
                annotations[key] = value
            if "qmtest.run.end_time" in annotations:
                del annotations["qmtest.run.end_time"]
        else:
            # All other annotations are added to a list
            if key in annotations:
                old = annotations[key]
                if type(old) is list:
                    if value not in old:
                        annotations[key].append(value)
                elif value != old:
                    annotations[key] = [old, value]
            else:
                annotations[key] = value
        # Write the new annotations file
        json.dump(annotations, open(self._annotationsFile, "w"),
                  sort_keys = True)

    def WriteResult(self, result):
        """Prepare the test result directory in the destination directory storing
        into it the result fields.
        A summary of the test result is stored both in a file in the test directory
        and in the global summary file.
        """
        summary = {}
        summary["id"] = result.GetId()
        summary["outcome"] = result.GetOutcome()
        summary["cause"] = result.GetCause()
        summary["fields"] = result.keys()
        summary["fields"].sort()

        # Since we miss proper JSON support, I hack a bit
        for f in ["id", "outcome", "cause"]:
            summary[f] = str(summary[f])
        summary["fields"] = map(str, summary["fields"])

        self._summary.append(summary)

        # format:
        # testname/summary.json
        # testname/field1
        # testname/field2
        testOutDir = os.path.join(self.dir, summary["id"])
        if not os.path.isdir(testOutDir):
            os.makedirs(testOutDir)
        json.dump(summary, open(os.path.join(testOutDir, "summary.json"), "w"),
                  sort_keys = True)
        for f in summary["fields"]:
            open(os.path.join(testOutDir, f), "w").write(result[f])

        self._updateSummary()

    def Summarize(self):
        # Not implemented.
        pass

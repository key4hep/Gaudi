# -*- coding: utf-8 -*-
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

import os
import sys
import time
import signal
import threading
import platform
import tempfile
import inspect
import re
import logging

from subprocess import Popen, PIPE, STDOUT

import six

if sys.version_info < (3, 5):
    # backport of 'backslashreplace' handling of UnicodeDecodeError
    # to Python < 3.5
    from codecs import register_error, backslashreplace_errors

    def _new_backslashreplace_errors(exc):
        if isinstance(exc, UnicodeDecodeError):
            code = hex(ord(exc.object[exc.start]))
            return (u'\\' + code[1:], exc.start + 1)
        else:
            return backslashreplace_errors(exc)

    register_error('backslashreplace', _new_backslashreplace_errors)
    del register_error
    del backslashreplace_errors
    del _new_backslashreplace_errors


def sanitize_for_xml(data):
    '''
    Take a string with invalid ASCII/UTF characters and quote them so that the
    string can be used in an XML text.

    >>> sanitize_for_xml('this is \x1b')
    'this is [NON-XML-CHAR-0x1B]'
    '''
    bad_chars = re.compile(
        u'[\x00-\x08\x0b\x0c\x0e-\x1F\uD800-\uDFFF\uFFFE\uFFFF]')

    def quote(match):
        'helper function'
        return ''.join('[NON-XML-CHAR-0x%2X]' % ord(c) for c in match.group())

    return bad_chars.sub(quote, data)


def dumpProcs(name):
    '''helper to debug GAUDI-1084, dump the list of processes'''
    from getpass import getuser
    if 'WORKSPACE' in os.environ:
        p = Popen(['ps', '-fH', '-U', getuser()], stdout=PIPE)
        with open(os.path.join(os.environ['WORKSPACE'], name), 'wb') as f:
            f.write(p.communicate()[0])


def kill_tree(ppid, sig):
    '''
    Send a signal to a process and all its child processes (starting from the
    leaves).
    '''
    log = logging.getLogger('kill_tree')
    ps_cmd = ['ps', '--no-headers', '-o', 'pid', '--ppid', str(ppid)]
    get_children = Popen(ps_cmd, stdout=PIPE, stderr=PIPE)
    children = map(int, get_children.communicate()[0].split())
    for child in children:
        kill_tree(child, sig)
    try:
        log.debug('killing process %d', ppid)
        os.kill(ppid, sig)
    except OSError as err:
        if err.errno != 3:  # No such process
            raise
        log.debug('no such process %d', ppid)


# -------------------------------------------------------------------------#


class BaseTest(object):

    _common_tmpdir = None

    def __init__(self):
        self.program = ''
        self.args = []
        self.reference = ''
        self.error_reference = ''
        self.options = ''
        self.stderr = ''
        self.timeout = 600
        self.exit_code = None
        self.environment = None
        self.unsupported_platforms = []
        self.signal = None
        self.workdir = os.curdir
        self.use_temp_dir = False
        # Variables not for users
        self.status = None
        self.name = ''
        self.causes = []
        self.result = Result(self)
        self.returnedCode = 0
        self.out = ''
        self.err = ''
        self.proc = None
        self.stack_trace = None
        self.basedir = os.getcwd()

    def run(self):
        logging.debug('running test %s', self.name)

        if self.options:
            if re.search(
                    r'from\s+Gaudi.Configuration\s+import\s+\*|'
                    'from\s+Configurables\s+import', self.options):
                optionFile = tempfile.NamedTemporaryFile(suffix='.py')
            else:
                optionFile = tempfile.NamedTemporaryFile(suffix='.opts')
            optionFile.file.write(self.options.encode('utf-8'))
            optionFile.seek(0)
            self.args.append(RationalizePath(optionFile.name))

        # If not specified, setting the environment
        if self.environment is None:
            self.environment = os.environ
        else:
            self.environment = dict(
                list(self.environment.items()) + list(os.environ.items()))

        platform_id = (os.environ.get('BINARY_TAG')
                       or os.environ.get('CMTCONFIG') or platform.platform())
        # If at least one regex matches we skip the test.
        skip_test = bool([
            None for prex in self.unsupported_platforms
            if re.search(prex, platform_id)
        ])

        if not skip_test:
            # handle working/temporary directory options
            workdir = self.workdir
            if self.use_temp_dir:
                if self._common_tmpdir:
                    workdir = self._common_tmpdir
                else:
                    workdir = tempfile.mkdtemp()

            # prepare the command to execute
            prog = ''
            if self.program != '':
                prog = self.program
            elif "GAUDIEXE" in os.environ:
                prog = os.environ["GAUDIEXE"]
            else:
                prog = "Gaudi.exe"

            dummy, prog_ext = os.path.splitext(prog)
            if prog_ext not in [".exe", ".py", ".bat"]:
                prog += ".exe"
                prog_ext = ".exe"

            prog = which(prog) or prog

            args = list(map(RationalizePath, self.args))

            if prog_ext == ".py":
                params = ['python', RationalizePath(prog)] + args
            else:
                params = [RationalizePath(prog)] + args

            validatorRes = Result({
                'CAUSE': None,
                'EXCEPTION': None,
                'RESOURCE': None,
                'TARGET': None,
                'TRACEBACK': None,
                'START_TIME': None,
                'END_TIME': None,
                'TIMEOUT_DETAIL': None
            })
            self.result = validatorRes

            # we need to switch directory because the validator expects to run
            # in the same dir as the program
            os.chdir(workdir)

            # launching test in a different thread to handle timeout exception
            def target():
                logging.debug('executing %r in %s', params, workdir)
                self.proc = Popen(
                    params, stdout=PIPE, stderr=PIPE, env=self.environment)
                logging.debug('(pid: %d)', self.proc.pid)
                out, err = self.proc.communicate()
                self.out = out.decode('utf-8', errors='backslashreplace')
                self.err = err.decode('utf-8', errors='backslashreplace')

            thread = threading.Thread(target=target)
            thread.start()
            # catching timeout
            thread.join(self.timeout)

            if thread.is_alive():
                logging.debug('time out in test %s (pid %d)', self.name,
                              self.proc.pid)
                # get the stack trace of the stuck process
                cmd = [
                    'gdb', '--pid',
                    str(self.proc.pid), '--batch',
                    '--eval-command=thread apply all backtrace'
                ]
                gdb = Popen(cmd, stdin=PIPE, stdout=PIPE, stderr=STDOUT)
                self.stack_trace = gdb.communicate()[0].decode(
                    'utf-8', errors='backslashreplace')

                kill_tree(self.proc.pid, signal.SIGTERM)
                thread.join(60)
                if thread.is_alive():
                    kill_tree(self.proc.pid, signal.SIGKILL)
                self.causes.append('timeout')
            else:
                logging.debug('completed test %s', self.name)

                # Getting the error code
                logging.debug('returnedCode = %s', self.proc.returncode)
                self.returnedCode = self.proc.returncode

                logging.debug('validating test...')
                self.result, self.causes = self.ValidateOutput(
                    stdout=self.out, stderr=self.err, result=validatorRes)

            # remove the temporary directory if we created it
            if self.use_temp_dir and not self._common_tmpdir:
                shutil.rmtree(workdir, True)

            os.chdir(self.basedir)

            # handle application exit code
            if self.signal is not None:
                if int(self.returnedCode) != -int(self.signal):
                    self.causes.append('exit code')

            elif self.exit_code is not None:
                if int(self.returnedCode) != int(self.exit_code):
                    self.causes.append('exit code')

            elif self.returnedCode != 0:
                self.causes.append("exit code")

            if self.causes:
                self.status = "failed"
            else:
                self.status = "passed"

        else:
            self.status = "skipped"

        logging.debug('%s: %s', self.name, self.status)
        field_mapping = {
            'Exit Code': 'returnedCode',
            'stderr': 'err',
            'Arguments': 'args',
            'Environment': 'environment',
            'Status': 'status',
            'stdout': 'out',
            'Program Name': 'program',
            'Name': 'name',
            'Validator': 'validator',
            'Output Reference File': 'reference',
            'Error Reference File': 'error_reference',
            'Causes': 'causes',
            # 'Validator Result': 'result.annotations',
            'Unsupported Platforms': 'unsupported_platforms',
            'Stack Trace': 'stack_trace'
        }
        resultDict = [(key, getattr(self, attr))
                      for key, attr in field_mapping.items()
                      if getattr(self, attr)]
        resultDict.append(('Working Directory',
                           RationalizePath(
                               os.path.join(os.getcwd(), self.workdir))))
        # print(dict(resultDict).keys())
        resultDict.extend(self.result.annotations.items())
        # print(self.result.annotations.keys())
        return dict(resultDict)

    # -------------------------------------------------#
    # ----------------Validating tool------------------#
    # -------------------------------------------------#

    def ValidateOutput(self, stdout, stderr, result):
        if not self.stderr:
            self.validateWithReference(stdout, stderr, result, self.causes)
        elif stderr.strip() != self.stderr.strip():
            self.causes.append('standard error')
        return result, self.causes

    def findReferenceBlock(self,
                           reference=None,
                           stdout=None,
                           result=None,
                           causes=None,
                           signature_offset=0,
                           signature=None,
                           id=None):
        """
            Given a block of text, tries to find it in the output. The block had to be identified by a signature line. By default, the first line is used as signature, or the line pointed to by signature_offset. If signature_offset points outside the block, a signature line can be passed as signature argument. Note: if 'signature' is None (the default), a negative signature_offset is interpreted as index in a list (e.g. -1 means the last line), otherwise the it is interpreted as the number of lines before the first one of the block the signature must appear. The parameter 'id' allow to distinguish between different calls to this function in the same validation code.
            """

        if reference is None:
            reference = self.reference
        if stdout is None:
            stdout = self.out
        if result is None:
            result = self.result
        if causes is None:
            causes = self.causes

        reflines = list(
            filter(None, map(lambda s: s.rstrip(), reference.splitlines())))
        if not reflines:
            raise RuntimeError("Empty (or null) reference")
        # the same on standard output
        outlines = list(
            filter(None, map(lambda s: s.rstrip(), stdout.splitlines())))

        res_field = "GaudiTest.RefBlock"
        if id:
            res_field += "_%s" % id

        if signature is None:
            if signature_offset < 0:
                signature_offset = len(reference) + signature_offset
            signature = reflines[signature_offset]
        # find the reference block in the output file
        try:
            pos = outlines.index(signature)
            outlines = outlines[pos - signature_offset:pos + len(reflines) -
                                signature_offset]
            if reflines != outlines:
                msg = "standard output"
                # I do not want 2 messages in causes if the function is called
                # twice
                if not msg in causes:
                    causes.append(msg)
                result[res_field + ".observed"] = result.Quote(
                    "\n".join(outlines))
        except ValueError:
            causes.append("missing signature")
        result[res_field + ".signature"] = result.Quote(signature)
        if len(reflines) > 1 or signature != reflines[0]:
            result[res_field + ".expected"] = result.Quote("\n".join(reflines))
        return causes

    def countErrorLines(self,
                        expected={
                            'ERROR': 0,
                            'FATAL': 0
                        },
                        stdout=None,
                        result=None,
                        causes=None):
        """
            Count the number of messages with required severity (by default ERROR and FATAL)
            and check if their numbers match the expected ones (0 by default).
            The dictionary "expected" can be used to tune the number of errors and fatals
            allowed, or to limit the number of expected warnings etc.
            """

        if stdout is None:
            stdout = self.out
        if result is None:
            result = self.result
        if causes is None:
            causes = self.causes

        # prepare the dictionary to record the extracted lines
        errors = {}
        for sev in expected:
            errors[sev] = []

        outlines = stdout.splitlines()
        from math import log10
        fmt = "%%%dd - %%s" % (int(log10(len(outlines) + 1)))

        linecount = 0
        for l in outlines:
            linecount += 1
            words = l.split()
            if len(words) >= 2 and words[1] in errors:
                errors[words[1]].append(fmt % (linecount, l.rstrip()))

        for e in errors:
            if len(errors[e]) != expected[e]:
                causes.append('%s(%d)' % (e, len(errors[e])))
                result["GaudiTest.lines.%s" % e] = result.Quote('\n'.join(
                    errors[e]))
                result["GaudiTest.lines.%s.expected#" % e] = result.Quote(
                    str(expected[e]))

        return causes

    def CheckTTreesSummaries(self,
                             stdout=None,
                             result=None,
                             causes=None,
                             trees_dict=None,
                             ignore=r"Basket|.*size|Compression"):
        """
            Compare the TTree summaries in stdout with the ones in trees_dict or in
            the reference file. By default ignore the size, compression and basket
            fields.
            The presence of TTree summaries when none is expected is not a failure.
            """
        if stdout is None:
            stdout = self.out
        if result is None:
            result = self.result
        if causes is None:
            causes = self.causes
        if trees_dict is None:
            lreference = self._expandReferenceFileName(self.reference)
            # call the validator if the file exists
            if lreference and os.path.isfile(lreference):
                trees_dict = findTTreeSummaries(open(lreference).read())
            else:
                trees_dict = {}

        from pprint import PrettyPrinter
        pp = PrettyPrinter()
        if trees_dict:
            result["GaudiTest.TTrees.expected"] = result.Quote(
                pp.pformat(trees_dict))
            if ignore:
                result["GaudiTest.TTrees.ignore"] = result.Quote(ignore)

        trees = findTTreeSummaries(stdout)
        failed = cmpTreesDicts(trees_dict, trees, ignore)
        if failed:
            causes.append("trees summaries")
            msg = "%s: %s != %s" % getCmpFailingValues(trees_dict, trees,
                                                       failed)
            result["GaudiTest.TTrees.failure_on"] = result.Quote(msg)
            result["GaudiTest.TTrees.found"] = result.Quote(pp.pformat(trees))

        return causes

    def CheckHistosSummaries(self,
                             stdout=None,
                             result=None,
                             causes=None,
                             dict=None,
                             ignore=None):
        """
            Compare the TTree summaries in stdout with the ones in trees_dict or in
            the reference file. By default ignore the size, compression and basket
            fields.
            The presence of TTree summaries when none is expected is not a failure.
            """
        if stdout is None:
            stdout = self.out
        if result is None:
            result = self.result
        if causes is None:
            causes = self.causes

        if dict is None:
            lreference = self._expandReferenceFileName(self.reference)
            # call the validator if the file exists
            if lreference and os.path.isfile(lreference):
                dict = findHistosSummaries(open(lreference).read())
            else:
                dict = {}

        from pprint import PrettyPrinter
        pp = PrettyPrinter()
        if dict:
            result["GaudiTest.Histos.expected"] = result.Quote(
                pp.pformat(dict))
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

    def validateWithReference(self,
                              stdout=None,
                              stderr=None,
                              result=None,
                              causes=None,
                              preproc=None):
        '''
        Default validation acti*on: compare standard output and error to the
        reference files.
        '''

        if stdout is None:
            stdout = self.out
        if stderr is None:
            stderr = self.err
        if result is None:
            result = self.result
        if causes is None:
            causes = self.causes

        # set the default output preprocessor
        if preproc is None:
            preproc = normalizeExamples
        # check standard output
        lreference = self._expandReferenceFileName(self.reference)
        # call the validator if the file exists
        if lreference and os.path.isfile(lreference):
            causes += ReferenceFileValidator(
                lreference, "standard output", "Output Diff",
                preproc=preproc)(stdout, result)
        elif lreference:
            causes += ["missing reference file"]
        # Compare TTree summaries
        causes = self.CheckTTreesSummaries(stdout, result, causes)
        causes = self.CheckHistosSummaries(stdout, result, causes)
        if causes and lreference:  # Write a new reference file for stdout
            try:
                cnt = 0
                newrefname = '.'.join([lreference, 'new'])
                while os.path.exists(newrefname):
                    cnt += 1
                    newrefname = '.'.join([lreference, '~%d~' % cnt, 'new'])
                newref = open(newrefname, "w")
                # sanitize newlines
                for l in stdout.splitlines():
                    newref.write(l.rstrip() + '\n')
                del newref  # flush and close
                result['New Output Reference File'] = os.path.relpath(
                    newrefname, self.basedir)
            except IOError:
                # Ignore IO errors when trying to update reference files
                # because we may be in a read-only filesystem
                pass

        # check standard error
        lreference = self._expandReferenceFileName(self.error_reference)
        # call the validator if we have a file to use
        if lreference:
            if os.path.isfile(lreference):
                newcauses = ReferenceFileValidator(
                    lreference,
                    "standard error",
                    "Error Diff",
                    preproc=preproc)(stderr, result)
            else:
                newcauses += ["missing error reference file"]
            causes += newcauses
            if newcauses and lreference:  # Write a new reference file for stdedd
                cnt = 0
                newrefname = '.'.join([lreference, 'new'])
                while os.path.exists(newrefname):
                    cnt += 1
                    newrefname = '.'.join([lreference, '~%d~' % cnt, 'new'])
                newref = open(newrefname, "w")
                # sanitize newlines
                for l in stderr.splitlines():
                    newref.write(l.rstrip() + '\n')
                del newref  # flush and close
                result['New Error Reference File'] = os.path.relpath(
                    newrefname, self.basedir)
        else:
            causes += BasicOutputValidator(lreference, "standard error",
                                           "ExecTest.expected_stderr")(stderr,
                                                                       result)
        return causes

    def _expandReferenceFileName(self, reffile):
        # if no file is passed, do nothing
        if not reffile:
            return ""

        # function to split an extension in constituents parts
        def platformSplit(p):
            import re
            delim = re.compile('-' in p and r"[-+]" or r"_")
            return set(delim.split(p))

        reference = os.path.normpath(
            os.path.join(self.basedir, os.path.expandvars(reffile)))

        # old-style platform-specific reference name
        spec_ref = reference[:-3] + GetPlatform(self)[0:3] + reference[-3:]
        if os.path.isfile(spec_ref):
            reference = spec_ref
        else:  # look for new-style platform specific reference files:
            # get all the files whose name start with the reference filename
            dirname, basename = os.path.split(reference)
            if not dirname:
                dirname = '.'
            head = basename + "."
            head_len = len(head)
            platform = platformSplit(GetPlatform(self))
            if 'do0' in platform:
                platform.add('dbg')
            candidates = []
            for f in os.listdir(dirname):
                if f.startswith(head):
                    req_plat = platformSplit(f[head_len:])
                    if platform.issuperset(req_plat):
                        candidates.append((len(req_plat), f))
            if candidates:  # take the one with highest matching
                # FIXME: it is not possible to say if x86_64-slc5-gcc43-dbg
                #        has to use ref.x86_64-gcc43 or ref.slc5-dbg
                candidates.sort()
                reference = os.path.join(dirname, candidates[-1][1])
        return reference


# ======= GAUDI TOOLS =======

import shutil
import string
import difflib
import calendar

try:
    from GaudiKernel import ROOT6WorkAroundEnabled
except ImportError:

    def ROOT6WorkAroundEnabled(id=None):
        # dummy implementation
        return False


# --------------------------------- TOOLS ---------------------------------#


def RationalizePath(p):
    """
    Function used to normalize the used path
    """
    newPath = os.path.normpath(os.path.expandvars(p))
    if os.path.exists(newPath):
        p = os.path.realpath(newPath)
    return p


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
            else:
                head, executable = os.path.split(executable)
        else:
            return executable
    for d in os.environ.get("PATH").split(os.pathsep):
        fullpath = os.path.join(d, executable)
        if os.path.exists(fullpath):
            return fullpath
    if executable.endswith('.exe'):
        return which(executable[:-4])
    return None


# -------------------------------------------------------------------------#
# ----------------------------- Result Classe -----------------------------#
# -------------------------------------------------------------------------#
import types


class Result:

    PASS = 'PASS'
    FAIL = 'FAIL'
    ERROR = 'ERROR'
    UNTESTED = 'UNTESTED'

    EXCEPTION = ""
    RESOURCE = ""
    TARGET = ""
    TRACEBACK = ""
    START_TIME = ""
    END_TIME = ""
    TIMEOUT_DETAIL = ""

    def __init__(self, kind=None, id=None, outcome=PASS, annotations={}):
        self.annotations = annotations.copy()

    def __getitem__(self, key):
        assert isinstance(key, six.string_types)
        return self.annotations[key]

    def __setitem__(self, key, value):
        assert isinstance(key, six.string_types)
        assert isinstance(
            value, six.string_types), '{!r} is not a string'.format(value)
        self.annotations[key] = value

    def Quote(self, string):
        return string


# -------------------------------------------------------------------------#
# --------------------------- Validator Classes ---------------------------#
# -------------------------------------------------------------------------#

# Basic implementation of an option validator for Gaudi test. This
# implementation is based on the standard (LCG) validation functions used
# in QMTest.


class BasicOutputValidator:
    def __init__(self, ref, cause, result_key):
        self.ref = ref
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
        # Check the output
        if not self.__CompareText(out, self.ref):
            causes.append(self.cause)
            result[self.result_key] = result.Quote(self.ref)

        return causes

    def __CompareText(self, s1, s2):
        """Compare 's1' and 's2', ignoring line endings.
            's1' -- A string.
            's2' -- A string.
            returns -- True if 's1' and 's2' are the same, ignoring
            differences in line endings."""
        if ROOT6WorkAroundEnabled('ReadRootmapCheck'):
            # FIXME: (MCl) Hide warnings from new rootmap sanity check until we
            # can fix them
            to_ignore = re.compile(
                r'Warning in <TInterpreter::ReadRootmapFile>: .* is already in .*'
            )

            def keep_line(l):
                return not to_ignore.match(l)

            return list(filter(keep_line, s1.splitlines())) == list(
                filter(keep_line, s2.splitlines()))
        else:
            return s1.splitlines() == s2.splitlines()


# ------------------------ Preprocessor elements ------------------------#
class FilePreprocessor:
    """ Base class for a callable that takes a file and returns a modified
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
        if not isinstance(input, six.string_types):
            lines = input
            mergeback = False
        else:
            lines = input.splitlines()
            mergeback = True
        output = self.__processFile__(lines)
        if mergeback:
            output = '\n'.join(output)
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
    "00:00:00 1970-01-01")
normalizeEOL = FilePreprocessor()
normalizeEOL.__processLine__ = lambda line: str(line).rstrip() + '\n'

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
            line = line[:(pos + self.siglen)]
            lst = line[(pos + self.siglen):].split()
            lst.sort()
            line += " ".join(lst)
        return line


class SortGroupOfLines(FilePreprocessor):
    '''
    Sort group of lines matching a regular expression
    '''

    def __init__(self, exp):
        self.exp = exp if hasattr(exp, 'match') else re.compile(exp)

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


# Preprocessors for GaudiExamples
normalizeExamples = maskPointers + normalizeDate
for w, o, r in [
        # ("TIMER.TIMER",r"[0-9]", "0"), # Normalize time output
    ("TIMER.TIMER", r"\s+[+-]?[0-9]+[0-9.]*", " 0"),  # Normalize time output
    ("release all pending", r"^.*/([^/]*:.*)", r"\1"),
    ("^#.*file", r"file '.*[/\\]([^/\\]*)$", r"file '\1"),
    ("^JobOptionsSvc.*options successfully read in from",
     r"read in from .*[/\\]([^/\\]*)$",
     r"file \1"),  # normalize path to options
        # Normalize UUID, except those ending with all 0s (i.e. the class IDs)
    (None,
     r"[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}(?!-0{12})-[0-9A-Fa-f]{12}",
     "00000000-0000-0000-0000-000000000000"),
        # Absorb a change in ServiceLocatorHelper
    ("ServiceLocatorHelper::", "ServiceLocatorHelper::(create|locate)Service",
     "ServiceLocatorHelper::service"),
        # Remove the leading 0 in Windows' exponential format
    (None, r"e([-+])0([0-9][0-9])", r"e\1\2"),
        # Output line changed in Gaudi v24
    (None, r'Service reference count check:',
     r'Looping over all active services...'),
        # Ignore count of declared properties (anyway they are all printed)
    (None,
     r"^(.*(DEBUG|SUCCESS) List of ALL properties of .*#properties = )\d+",
     r"\1NN"),
    ('ApplicationMgr', r'(declareMultiSvcType|addMultiSvc): ', ''),
    (r"Property \['Name': Value\]", r"( =  '[^']+':)'(.*)'", r'\1\2'),
    ('TimelineSvc', "to file  'TimelineFile':", "to file "),
    ('DataObjectHandleBase', r'DataObjectHandleBase\("([^"]*)"\)', r"'\1'"),
]:  # [ ("TIMER.TIMER","[0-9]+[0-9.]*", "") ]
    normalizeExamples += RegexpReplacer(o, r, w)

lineSkipper = LineSkipper(
    [
        "//GP:",
        "JobOptionsSvc        INFO # ",
        "JobOptionsSvc     WARNING # ",
        "Time User",
        "Welcome to",
        "This machine has a speed",
        "TIME:",
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
        'SIGXCPU',
        # Message removed with redesing of JobOptionsSvc
        'ServiceLocatorHelper::service: found service JobOptionsSvc',
        # Ignore warnings for properties case mismatch
        'mismatching case for property name:',
        # Message demoted to DEBUG in gaudi/Gaudi!992
        'Histograms saving not required.',
        # Message added in gaudi/Gaudi!577
        'Properties are dumped into',
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
        # I want to ignore the header of the unchecked StatusCode report
        r"^StatusCodeSvc.*listing all unchecked return codes:",
        r"^StatusCodeSvc\s*INFO\s*$",
        r"Num\s*\|\s*Function\s*\|\s*Source Library",
        r"^[-+]*\s*$",
        # Hide the fake error message coming from POOL/ROOT (ROOT 5.21)
        r"ERROR Failed to modify file: .* Errno=2 No such file or directory",
        # Hide unchecked StatusCodes from dictionaries
        r"^ +[0-9]+ \|.*ROOT",
        r"^ +[0-9]+ \|.*\|.*Dict",
        # Hide success StatusCodeSvc message
        r"StatusCodeSvc.*all StatusCode instances where checked",
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
    ])

if ROOT6WorkAroundEnabled('ReadRootmapCheck'):
    # FIXME: (MCl) Hide warnings from new rootmap sanity check until we can
    # fix them
    lineSkipper += LineSkipper(regexps=[
        r'Warning in <TInterpreter::ReadRootmapFile>: .* is already in .*',
    ])

normalizeExamples = (
    lineSkipper + normalizeExamples + skipEmptyLines + normalizeEOL +
    LineSorter("Services to release : ") +
    SortGroupOfLines(r'^\S+\s+(DEBUG|SUCCESS) Property \[\'Name\':'))

# --------------------- Validation functions/classes ---------------------#


class ReferenceFileValidator:
    def __init__(self, reffile, cause, result_key, preproc=normalizeExamples):
        self.reffile = os.path.expandvars(reffile)
        self.cause = cause
        self.result_key = result_key
        self.preproc = preproc

    def __call__(self, stdout, result):
        causes = []
        if os.path.isfile(self.reffile):
            orig = open(self.reffile).readlines()
            if self.preproc:
                orig = self.preproc(orig)
                result[self.result_key + '.preproc.orig'] = \
                    result.Quote('\n'.join(map(str.strip, orig)))
        else:
            orig = []
        new = stdout.splitlines()
        if self.preproc:
            new = self.preproc(new)

        diffs = difflib.ndiff(orig, new, charjunk=difflib.IS_CHARACTER_JUNK)
        filterdiffs = list(
            map(lambda x: x.strip(), filter(lambda x: x[0] != " ", diffs)))
        if filterdiffs:
            result[self.result_key] = result.Quote("\n".join(filterdiffs))
            result[self.result_key] += result.Quote("""
                Legend:
                -) reference file
                +) standard output of the test""")
            result[self.result_key + '.preproc.new'] = \
                result.Quote('\n'.join(map(str.strip, new)))
            causes.append(self.cause)
        return causes


def findTTreeSummaries(stdout):
    """
        Scan stdout to find ROOT TTree summaries and digest them.
        """
    stars = re.compile(r"^\*+$")
    outlines = stdout.splitlines()
    nlines = len(outlines)
    trees = {}

    i = 0
    while i < nlines:  # loop over the output
        # look for
        while i < nlines and not stars.match(outlines[i]):
            i += 1
        if i < nlines:
            tree, i = _parseTTreeSummary(outlines, i)
            if tree:
                trees[tree["Name"]] = tree

    return trees


def cmpTreesDicts(reference, to_check, ignore=None):
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
        keys = [key for key in reference if not ignore_re.match(key)]
    else:
        keys = reference.keys()
    # loop over the keys (not ignored) in the reference dictionary
    for k in keys:
        if k in to_check:  # the key must be in the dictionary to_check
            if (type(reference[k]) is dict) and (type(to_check[k]) is dict):
                # if both reference and to_check values are dictionaries,
                # recurse
                failed = fail_keys = cmpTreesDicts(reference[k], to_check[k],
                                                   ignore)
            else:
                # compare the two values
                failed = to_check[k] != reference[k]
        else:  # handle missing keys in the dictionary to check (i.e. failure)
            to_check[k] = None
            failed = True
        if failed:
            fail_keys.insert(0, k)
            break  # exit from the loop at the first failure
    return fail_keys  # return the list of keys bringing to the different values


def getCmpFailingValues(reference, to_check, fail_path):
    c = to_check
    r = reference
    for k in fail_path:
        c = c.get(k, None)
        r = r.get(k, None)
        if c is None or r is None:
            break  # one of the dictionaries is not deep enough
    return (fail_path, r, c)


# signature of the print-out of the histograms
h_count_re = re.compile(
    r"^(.*)SUCCESS\s+Booked (\d+) Histogram\(s\) :\s+([\s\w=-]*)")


def _parseTTreeSummary(lines, pos):
    """
    Parse the TTree summary table in lines, starting from pos.
    Returns a tuple with the dictionary with the digested informations and the
    position of the first line after the summary.
    """
    result = {}
    i = pos + 1  # first line is a sequence of '*'
    count = len(lines)

    def splitcols(l):
        return [f.strip() for f in l.strip("*\n").split(':', 2)]

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
        result = parseblock(lines[i:i + 3])
        result["Branches"] = {}
        i += 4
        while i < (count - 3) and lines[i].startswith("*Br"):
            if i < (count - 2) and lines[i].startswith("*Branch "):
                # skip branch header
                i += 3
                continue
            branch = parseblock(lines[i:i + 3])
            result["Branches"][branch["Name"]] = branch
            i += 4

    return (result, i)


def parseHistosSummary(lines, pos):
    """
        Extract the histograms infos from the lines starting at pos.
        Returns the position of the first line after the summary block.
        """
    global h_count_re
    h_table_head = re.compile(
        r'SUCCESS\s+(1D|2D|3D|1D profile|2D profile) histograms in directory\s+"(\w*)"'
    )
    h_short_summ = re.compile(r"ID=([^\"]+)\s+\"([^\"]+)\"\s+(.*)")

    nlines = len(lines)

    # decode header
    m = h_count_re.search(lines[pos])
    name = m.group(1).strip()
    total = int(m.group(2))
    header = {}
    for k, v in [x.split("=") for x in m.group(3).split()]:
        header[k] = int(v)
    pos += 1
    header["Total"] = total

    summ = {}
    while pos < nlines:
        m = h_table_head.search(lines[pos])
        if m:
            t, d = m.groups(1)  # type and directory
            t = t.replace(" profile", "Prof")
            pos += 1
            if pos < nlines:
                l = lines[pos]
            else:
                l = ""
            cont = {}
            if l.startswith(" | ID"):
                # table format
                titles = [x.strip() for x in l.split("|")][1:]
                pos += 1
                while pos < nlines and lines[pos].startswith(" |"):
                    l = lines[pos]
                    values = [x.strip() for x in l.split("|")][1:]
                    hcont = {}
                    for i in range(len(titles)):
                        hcont[titles[i]] = values[i]
                    cont[hcont["ID"]] = hcont
                    pos += 1
            elif l.startswith(" ID="):
                while pos < nlines and lines[pos].startswith(" ID="):
                    values = [
                        x.strip()
                        for x in h_short_summ.search(lines[pos]).groups()
                    ]
                    cont[values[0]] = values
                    pos += 1
            else:  # not interpreted
                raise RuntimeError(
                    "Cannot understand line %d: '%s'" % (pos, l))
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


def PlatformIsNotSupported(self, context, result):
    platform = GetPlatform(self)
    unsupported = [
        re.compile(x) for x in [str(y).strip() for y in unsupported_platforms]
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
    if "BINARY_TAG" in os.environ:
        arch = os.environ["BINARY_TAG"]
    elif "CMTCONFIG" in os.environ:
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
    platform = GetPlatform(self)
    return "winxp" in platform or platform.startswith("win")

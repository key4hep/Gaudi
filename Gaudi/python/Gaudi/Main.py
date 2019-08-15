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
from __future__ import print_function
import sys
import os
from time import time
from Gaudi import Configuration
import logging

import six

log = logging.getLogger(__name__)


class BootstrapHelper(object):
    class StatusCode(object):
        def __init__(self, value):
            self.value = value

        def __bool__(self):
            return self.value

        __nonzero__ = __bool__

        def isSuccess(self):
            return self.value

        def isFailure(self):
            return not self.value

        def ignore(self):
            pass

    class Property(object):
        def __init__(self, value):
            self.value = value

        def __str__(self):
            # TODO why bytes
            return bytes(self.value).decode('utf-8')

        toString = __str__

    class AppMgr(object):
        def __init__(self, ptr, lib):
            self.ptr = ptr
            self.lib = lib
            self._as_parameter_ = ptr

        def configure(self):
            return BootstrapHelper.StatusCode(
                self.lib.py_bootstrap_fsm_configure(self.ptr))

        def initialize(self):
            return BootstrapHelper.StatusCode(
                self.lib.py_bootstrap_fsm_initialize(self.ptr))

        def start(self):
            return BootstrapHelper.StatusCode(
                self.lib.py_bootstrap_fsm_start(self.ptr))

        def run(self, nevt):
            return BootstrapHelper.StatusCode(
                self.lib.py_bootstrap_app_run(self.ptr, nevt))

        def stop(self):
            return BootstrapHelper.StatusCode(
                self.lib.py_bootstrap_fsm_stop(self.ptr))

        def finalize(self):
            return BootstrapHelper.StatusCode(
                self.lib.py_bootstrap_fsm_finalize(self.ptr))

        def terminate(self):
            return BootstrapHelper.StatusCode(
                self.lib.py_bootstrap_fsm_terminate(self.ptr))

        def getService(self, name):
            return self.lib.py_bootstrap_getService(self.ptr,
                                                    name.encode('ascii'))

        def setProperty(self, name, value):
            return BootstrapHelper.StatusCode(
                self.lib.py_bootstrap_setProperty(self.ptr,
                                                  name.encode('ascii'),
                                                  value.encode('ascii')))

        def getProperty(self, name):
            return BootstrapHelper.Property(
                self.lib.py_bootstrap_getProperty(self.ptr,
                                                  name.encode('ascii')))

        def printAlgsSequences(self):
            return self.lib.py_helper_printAlgsSequences(self.ptr)

    def __init__(self):
        from ctypes import (PyDLL, util, c_void_p, c_bool, c_char_p, c_int,
                            RTLD_GLOBAL)

        # Helper class to avoid void* to int conversion
        # (see http://stackoverflow.com/questions/17840144)

        class IInterface_p(c_void_p):
            def __repr__(self):
                return "IInterface_p(0x%x)" % (0 if self.value is None else
                                               self.value)

        self.log = logging.getLogger('BootstrapHelper')
        libname = util.find_library('GaudiKernel') or 'libGaudiKernel.so'
        self.log.debug('loading GaudiKernel (%s)', libname)

        # FIXME: note that we need PyDLL instead of CDLL if the calls to
        #        Python functions are not protected with the GIL.
        self.lib = gkl = PyDLL(libname, mode=RTLD_GLOBAL)

        functions = [
            ('createApplicationMgr', IInterface_p, []),
            ('getService', IInterface_p, [IInterface_p, c_char_p]),
            ('setProperty', c_bool, [IInterface_p, c_char_p, c_char_p]),
            ('getProperty', c_char_p, [IInterface_p, c_char_p]),
            ('addPropertyToCatalogue', c_bool,
             [IInterface_p, c_char_p, c_char_p, c_char_p]),
            ('ROOT_VERSION_CODE', c_int, []),
        ]

        for name, restype, argtypes in functions:
            f = getattr(gkl, 'py_bootstrap_%s' % name)
            f.restype, f.argtypes = restype, argtypes
            # create a delegate method if not already present
            # (we do not want to use hasattr because it calls "properties")
            if name not in self.__class__.__dict__:
                setattr(self, name, f)

        for name in ('configure', 'initialize', 'start', 'stop', 'finalize',
                     'terminate'):
            f = getattr(gkl, 'py_bootstrap_fsm_%s' % name)
            f.restype, f.argtypes = c_bool, [IInterface_p]
        gkl.py_bootstrap_app_run.restype = c_bool
        gkl.py_bootstrap_app_run.argtypes = [IInterface_p, c_int]

        gkl.py_helper_printAlgsSequences.restype = None
        gkl.py_helper_printAlgsSequences.argtypes = [IInterface_p]

    def createApplicationMgr(self):
        ptr = self.lib.py_bootstrap_createApplicationMgr()
        return self.AppMgr(ptr, self.lib)

    @property
    def ROOT_VERSION_CODE(self):
        return self.lib.py_bootstrap_ROOT_VERSION_CODE()

    @property
    def ROOT_VERSION(self):
        root_version_code = self.ROOT_VERSION_CODE
        a = root_version_code >> 16 & 0xff
        b = root_version_code >> 8 & 0xff
        c = root_version_code & 0xff
        return (a, b, c)


def getAllOpts(explicit_defaults=False):
    from itertools import chain
    # old conf
    from GaudiKernel.Proxy.Configurable import Configurable, getNeededConfigurables
    old_opts = {}

    # some algorithms may be generater when we call "getValuedProperties"
    # so we need a few iterations before we get the full list
    # (see GaudiConfig.ControlFlow)
    needed_conf = []
    count = 0
    new_count = -1
    while count != new_count:
        count = new_count
        needed_conf = getNeededConfigurables()
        new_count = len(needed_conf)
        for n in needed_conf:
            c = Configurable.allConfigurables[n]
            if hasattr(c, 'getValuedProperties'):
                c.getValuedProperties()

    for n in needed_conf:
        c = Configurable.allConfigurables[n]
        items = (chain(c.getDefaultProperties().items(),
                       c.getValuedProperties().items())
                 if explicit_defaults else c.getValuedProperties().items())
        for p, v in items:
            # Note: AthenaCommon.Configurable does not have Configurable.PropertyReference
            if hasattr(Configurable, "PropertyReference") and type(
                    v) == Configurable.PropertyReference:
                # this is done in "getFullName", but the exception is ignored,
                # so we do it again to get it
                v = v.__resolve__()
            if isinstance(v, str):
                # properly escape quotes in the string (see gaudi/Gaudi#78)
                v = '"%s"' % v.replace('"', '\\"')
            elif sys.version_info < (3, ) and isinstance(v, long):
                v = '%d' % v  # prevent pending 'L'
            elif hasattr(v, '__opt_value__'):
                v = v.__opt_value__()
            old_opts['.'.join((n, p))] = str(v)

    import GaudiConfig2
    opts = GaudiConfig2.all_options(explicit_defaults)

    conflicts = [
        n for n in set(opts).intersection(old_opts) if opts[n] != old_opts[n]
    ]
    if conflicts:
        conflicts.sort()
        log.error('Some properties are set in old and new style configuration')
        log.warning('name: old -> new')
        for n in conflicts:
            log.warning('%s: %s -> %s', n, old_opts[n], opts[n])
            sys.exit(10)

    opts.update(old_opts)
    return opts


def toOpt(value):
    '''
    Helper to convert values to old .opts format.

    >>> print(toOpt('some "text"'))
    "some \\"text\\""
    >>> print(toOpt('first\\nsecond'))
    "first
    second"
    >>> print(toOpt({'a': [1, 2, '3']}))
    {"a": [1, 2, "3"]}
    '''
    if isinstance(value, six.string_types):
        return '"{0}"'.format(value.replace('"', '\\"'))
    elif isinstance(value, dict):
        return '{{{0}}}'.format(', '.join(
            '{0}: {1}'.format(toOpt(k), toOpt(v)) for k, v in value.items()))
    elif hasattr(value, '__iter__'):
        return '[{0}]'.format(', '.join(map(toOpt, value)))
    else:
        return repr(value)


def parseOpt(s):
    '''
    Helper to parse option strings to Python values.

    Ideally it should just be "eval", but the string parser of Gaudi
    is different from the Python one, so we get string options that
    cannot be just evaluated.

    >>> print(parseOpt('123'))
    123
    >>> print(parseOpt('"some\\n\\\\"text\\\\""'))
    some
    "text"
    >>> print(parseOpt(''))
    <BLANKLINE>

    (see gaudi/Gaudi#78)
    '''
    import re
    quoted_string = re.compile(r'^"(.*)"$', re.DOTALL)
    # FIXME: this is needed because we cannot use repr for strings
    #        (see gaudi/Gaudi#78)
    if not s:  # pass through empty strings
        return s
    m = quoted_string.match(s)
    if m:
        return m.group(1).replace('\\"', '"')
    return eval(s)


class gaudimain(object):
    def __init__(self):
        from Configurables import ApplicationMgr
        appMgr = ApplicationMgr()
        if "GAUDIAPPNAME" in os.environ:
            appMgr.AppName = str(os.environ["GAUDIAPPNAME"])
        if "GAUDIAPPVERSION" in os.environ:
            appMgr.AppVersion = str(os.environ["GAUDIAPPVERSION"])
        self.log = logging.getLogger(__name__)
        self.printsequence = False
        self.application = 'Gaudi::Application'

    def setupParallelLogging(self):
        # ---------------------------------------------------
        # set up Logging
        # ----------------
        # from multiprocessing import enableLogging, getLogger
        import multiprocessing
        # preliminaries for handlers/output files, etc.
        from time import ctime
        datetime = ctime()
        datetime = datetime.replace(' ', '_')
        outfile = open('gaudirun-%s.log' % (datetime), 'w')
        # two handlers, one for a log file, one for terminal
        streamhandler = logging.StreamHandler(stream=outfile)
        console = logging.StreamHandler()
        # create formatter : the params in parentheses are variable names available via logging
        formatter = logging.Formatter(
            "%(asctime)s - %(name)s - %(levelname)s - %(message)s")
        # add formatter to Handler
        streamhandler.setFormatter(formatter)
        console.setFormatter(formatter)
        # now, configure the logger
        # enableLogging( level=0 )
        # self.log = getLogger()
        self.log = multiprocessing.log_to_stderr()
        self.log.setLevel(logging.INFO)
        self.log.name = 'Gaudi/Main.py Logger'
        self.log.handlers = []
        # add handlers to logger : one for output to a file, one for console output
        self.log.addHandler(streamhandler)
        self.log.addHandler(console)
        self.log.removeHandler(console)
        # set level!!
        self.log.setLevel = logging.INFO
        # ---------------------------------------------------

    def generatePyOutput(self, all=False):
        from pprint import pformat
        from collections import defaultdict
        optDict = defaultdict(dict)
        allOpts = getAllOpts(all)
        for key in allOpts:
            c, p = key.rsplit('.', 1)
            optDict[c][p] = parseOpt(allOpts[key])
        formatted = pformat(dict(optDict))
        # Python 2 compatibility
        if six.PY2:
            return formatted
        else:
            # undo splitting of strings on multiple lines
            import re
            return re.sub(r'"\n +"', '', formatted, flags=re.MULTILINE)

    def generateOptsOutput(self, all=False):
        opts = getAllOpts(all)
        keys = sorted(opts)
        return '\n'.join(
            '{} = {};'.format(key, toOpt(parseOpt(opts[key]))) for key in keys)

    def _writepickle(self, filename):
        # --- Lets take the first file input file as the name of the pickle file
        import pickle
        output = open(filename, 'wb')
        # Dump only the the configurables that make sense to dump (not User ones)
        from GaudiKernel.Proxy.Configurable import getNeededConfigurables
        to_dump = {}
        for n in getNeededConfigurables():
            to_dump[n] = Configuration.allConfigurables[n]
        pickle.dump(to_dump, output, -1)
        output.close()

    def printconfig(self, old_format=False, all=False):
        msg = 'Dumping all configurables and properties'
        if not all:
            msg += ' (different from default)'
        log.info(msg)
        if old_format:
            print(self.generateOptsOutput(all))
        else:
            print(self.generatePyOutput(all))
        sys.stdout.flush()

    def writeconfig(self, filename, all=False):
        write = {".pkl": lambda filename, all: self._writepickle(filename),
                 ".py": lambda filename, all: open(filename, "w").write(self.generatePyOutput(all) + "\n"),
                 ".opts": lambda filename, all: open(filename, "w").write(self.generateOptsOutput(all) + "\n"),
                 }
        from os.path import splitext
        ext = splitext(filename)[1]
        if ext in write:
            write[ext](filename, all)
        else:
            log.error("Unknown file type '%s'. Must be any of %r.", ext,
                      write.keys())
            sys.exit(1)

    # Instantiate and run the application.
    #  Depending on the number of CPUs (ncpus) specified, it start
    def run(self, attach_debugger, ncpus=None):
        if not ncpus:
            # Standard sequential mode
            result = self.runSerial(attach_debugger)
        else:
            # Otherwise, run with the specified number of cpus
            result = self.runParallel(ncpus)
        return result

    def hookDebugger(self, debugger='gdb'):
        import os
        self.log.info('attaching debugger to PID ' + str(os.getpid()))
        pid = os.spawnvp(os.P_NOWAIT, debugger,
                         [debugger, '-q', 'python',
                          str(os.getpid())])

        # give debugger some time to attach to the python process
        import time
        time.sleep(5)

        # verify the process' existence (will raise OSError if failed)
        os.waitpid(pid, os.WNOHANG)
        os.kill(pid, 0)
        return

    def runSerial(self, attach_debugger):
        try:
            from GaudiKernel.Proxy.Configurable import expandvars
        except ImportError:
            # pass-through implementation if expandvars is not defined (AthenaCommon)
            def expandvars(data):
                return data

        from GaudiKernel.Proxy.Configurable import Configurable

        self.log.debug('runSerial: apply options')
        conf_dict = expandvars(getAllOpts())
        conf_dict['ApplicationMgr.JobOptionsType'] = '"NONE"'

        if self.printsequence:
            conf_dict['ApplicationMgr.PrintAlgsSequence'] = 'true'

        if hasattr(Configurable, "_configurationLocked"):
            Configurable._configurationLocked = True

        if attach_debugger:
            self.hookDebugger()

        self.log.debug('-' * 80)
        self.log.debug('%s: running in serial mode', __name__)
        self.log.debug('-' * 80)
        sysStart = time()

        import Gaudi
        app = Gaudi.Application.create(self.application, conf_dict)
        retcode = app.run()

        sysTime = time() - sysStart
        self.log.debug('-' * 80)
        self.log.debug('%s: serial system finished, time taken: %5.4fs',
                       __name__, sysTime)
        self.log.debug('-' * 80)

        return retcode

    def runParallel(self, ncpus):
        self.setupParallelLogging()
        from Gaudi.Configuration import Configurable
        import GaudiMP.GMPBase as gpp
        c = Configurable.allConfigurables
        self.log.info('-' * 80)
        self.log.info('%s: Parallel Mode : %i ', __name__, ncpus)
        for name, value in [
            ('platform', ' '.join(os.uname())),
            ('config', os.environ.get('BINARY_TAG')
             or os.environ.get('CMTCONFIG')),
            ('app. name', os.environ.get('GAUDIAPPNAME')),
            ('app. version', os.environ.get('GAUDIAPPVERSION')),
        ]:
            self.log.info('%s: %30s : %s ', __name__, name, value
                          or 'Undefined')
        try:
            events = str(c['ApplicationMgr'].EvtMax)
        except:
            events = "Undetermined"
        self.log.info('%s: Events Specified : %s ', __name__, events)
        self.log.info('-' * 80)
        # Parall = gpp.Coordinator(ncpus, shared, c, self.log)
        Parall = gpp.Coord(ncpus, c, self.log)
        sysStart = time()
        sc = Parall.Go()
        self.log.info('MAIN.PY : received %s from Coordinator' % (sc))
        if sc.isFailure():
            return 1
        sysTime = time() - sysStart
        self.log.name = 'Gaudi/Main.py Logger'
        self.log.info('-' * 80)
        self.log.info('%s: parallel system finished, time taken: %5.4fs',
                      __name__, sysTime)
        self.log.info('-' * 80)
        return 0

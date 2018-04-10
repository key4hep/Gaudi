#!/usr/bin/env python

import os
import sys
from tempfile import mkstemp


def getArgsWithoutoProfilerInfo(args):
    """
    Remove from the arguments the presence of the profiler and its output in
    order to relaunch the script w/o infinite loops.

    >>> getArgsWithoutoProfilerInfo(['--profilerName', 'igprof', 'myopts.py'])
    ['myopts.py']

    >>> getArgsWithoutoProfilerInfo(['--profilerName=igprof', 'myopts.py'])
    ['myopts.py']

    >>> getArgsWithoutoProfilerInfo(['--profilerName', 'igprof', '--profilerExtraOptions', 'a b c', 'myopts.py'])
    ['myopts.py']

    >>> getArgsWithoutoProfilerInfo(['--profilerName', 'igprof', '--options', 'a b c', 'myopts.py'])
    ['--options', 'a b c', 'myopts.py']
    """
    newargs = []
    args = list(args)  # make a temp copy
    while args:
        o = args.pop(0)
        if o.startswith('--profile'):
            if '=' not in o:
                args.pop(0)
        else:
            newargs.append(o)
    return newargs


def setLibraryPreload(newpreload):
    ''' Adds  a list of libraries to LD_PRELOAD '''
    preload = os.environ.get("LD_PRELOAD", "")
    if preload:
        preload = preload.replace(" ", ":").split(":")
    else:
        preload = []

    for libname in set(preload).intersection(newpreload):
        logging.warning("Ignoring preload of library %s because it is "
                        "already in LD_PRELOAD.", libname)

    to_load = [libname
               for libname in newpreload
               if libname not in set(preload)]

    if to_load:
        preload += to_load
        preload = ":".join(preload)
        os.environ["LD_PRELOAD"] = preload
        logging.info("Setting LD_PRELOAD='%s'", preload)

    return to_load


def rationalizepath(path):
    '''
    Convert the given path to a real path if the pointed file exists, otherwise
    just normalize it.
    '''
    path = os.path.normpath(os.path.expandvars(path))
    if os.path.exists(path):
        path = os.path.realpath(path)
    return path


# variable used to keep alive the temporary option files extracted
# from the .qmt
_qmt_tmp_opt_files = []


def getArgsFromQmt(qmtfile):
    '''
    Given a .qmt file, return the command line arguments of the corresponding
    test.
    '''
    from xml.etree import ElementTree as ET
    global _qmt_tmp_opt_files
    # parse the .qmt file and extract args and options
    qmt = ET.parse(qmtfile)
    args = [a.text for a in qmt.findall("argument[@name='args']//text")]
    options = qmt.find("argument[@name='options']/text")

    if options is not None:  # options need to be dumped in a temporary file
        from tempfile import NamedTemporaryFile
        import re
        if re.search(r"from\s+Gaudi.Configuration\s+import\s+\*"
                     r"|from\s+Configurables\s+import", options.text):
            tmp_opts = NamedTemporaryFile(suffix='.py')
        else:
            tmp_opts = NamedTemporaryFile(suffix='.opts')
        tmp_opts.write(options.text)
        tmp_opts.flush()
        args.append(tmp_opts.name)
        _qmt_tmp_opt_files.append(tmp_opts)

    # relative paths in a .qmt are rooted in the qmtest directory, so
    # - find where the .qmt lives
    qmtfile = os.path.abspath(qmtfile)
    if 'qmtest' in qmtfile.split(os.path.sep):
        # this return the path up to the 'qmtest' entry in qmtfile
        testdir = qmtfile
        while os.path.basename(testdir) != 'qmtest':
            testdir = os.path.dirname(testdir)
    else:
        testdir = '.'
    # - temporarily switch to that directory and rationalize the paths
    old_cwd = os.getcwd()
    os.chdir(testdir)
    args = map(rationalizepath, args)
    os.chdir(old_cwd)

    return args


# ---------------------------------------------------------------------
if __name__ == "__main__":
    # ensure that we (and the subprocesses) use the C standard localization
    if os.environ.get('LC_ALL') != 'C':
        print '# setting LC_ALL to "C"'
        os.environ['LC_ALL'] = 'C'

    from optparse import OptionParser
    parser = OptionParser(usage="%prog [options] <opts_file> ...")
    parser.add_option("-n", "--dry-run", action="store_true",
                      help="do not run the application, just parse option files")
    parser.add_option("-p", "--pickle-output", action="store", type="string",
                      metavar="FILE",
                      help="DEPRECATED: use '--output file.pkl' instead. Write "
                           "the parsed options as a pickle file (static option "
                           "file)")
    parser.add_option("-v", "--verbose", action="store_true",
                      help="print the parsed options")
    parser.add_option("--old-opts", action="store_true",
                      help="format printed options in old option files style")
    parser.add_option("--all-opts", action="store_true",
                      help="print all the option (even if equal to default)")
    # GaudiPython Parallel Mode Option
    #   Argument must be an integer in range [ -1, sys_cpus ]
    #   -1   : All available cpus
    #    0   : Serial Mode (traditional gaudirun)
    #    n>0 : parallel with n cpus (n <= sys_cpus)
    parser.add_option("--ncpus", action="store", type="int", default=0,
                      help="start the application in parallel mode using NCPUS processes. "
                           "0 => serial mode (default), -1 => use all CPUs")

    def option_cb(option, opt, value, parser):
        """Add the option line to a list together with its position in the
        argument list.
        """
        parser.values.options.append((len(parser.largs), value))
    parser.add_option("--option", action="callback", callback=option_cb,
                      type="string", nargs=1,
                      help="add a single line (Python) option to the configuration. "
                           "All options lines are executed, one after the other, in "
                           "the same context.")
    parser.add_option("--no-conf-user-apply", action="store_true",
                      help="disable the automatic application of configurable "
                           "users (for backward compatibility)")
    parser.add_option("--old-conf-user-apply", action="store_true",
                      help="use the old logic when applying ConfigurableUsers "
                           "(with bug #103803) [default]")
    parser.add_option("--new-conf-user-apply", action="store_false",
                      dest="old_conf_user_apply",
                      help="use the new (correct) logic when applying "
                           "ConfigurableUsers (fixed bug #103803), can be "
                           "turned on also with the environment variable "
                           "GAUDI_FIXED_APPLY_CONF")
    parser.add_option("-o", "--output", action="store", type="string",
                      help="dump the configuration to a file. The format of "
                            "the options is determined by the extension of the "
                            "file name: .pkl = pickle, .py = python, .opts = "
                            "old style options. The python format cannot be "
                            "used to run the application and it contains the "
                            "same dictionary printed with -v")
    parser.add_option("--post-option", action="append", type="string",
                      dest="post_options",
                      help="Python options to be executed after the ConfigurableUser "
                           "are applied. "
                           "All options lines are executed, one after the other, in "
                           "the same context.")
    parser.add_option("--debug", action="store_true",
                      help="enable some debug print-out")
    parser.add_option("--gdb", action="store_true",
                      help="attach gdb")
    parser.add_option("--printsequence", action="store_true",
                      help="print the sequence")
    if not sys.platform.startswith("win"):
        # These options can be used only on unix platforms
        parser.add_option("-T", "--tcmalloc", action="store_true",
                          help="Use the Google malloc replacement. The environment "
                               "variable TCMALLOCLIB can be used to specify a different "
                               "name for the library (the default is libtcmalloc.so)")
        parser.add_option("--preload", action="append",
                          help="Allow pre-loading of special libraries (e.g. Google "
                               "profiling libraries).")
        # Option to use a profiler
        parser.add_option("--profilerName", type="string",
                          help="Select one profiler among: igprofPerf, igprofMem and valgrind<toolname>")

        # Option to specify the filename where to collect the profiler's output
        parser.add_option("--profilerOutput", type="string",
                          help="Specify the name of the output file for the profiler output")

        # Option to specify the filename where to collect the profiler's output
        parser.add_option("--profilerExtraOptions", type="string",
                          help="Specify additional options for the profiler. The '--' string should be expressed as '__' (--my-opt becomes __my-opt)")

    parser.add_option('--use-temp-opts', action='store_true',
                      help='when this option is enabled, the options are parsed'
                           ' and stored in a temporary file, then the job is '
                           'restarted using that file as input (to save '
                           'memory)')
    parser.add_option("--run-info-file", type="string",
                      help="Save gaudi process information to the file specified (in JSON format)")

    parser.set_defaults(options=[],
                        tcmalloc=False,
                        profilerName='',
                        profilerOutput='',
                        profilerExtraOptions='',
                        preload=[],
                        ncpus=None,
                        # the old logic can be turned off with an env variable
                        old_conf_user_apply='GAUDI_FIXED_APPLY_CONF' not in os.environ,
                        run_info_file=None)

    # replace .qmt files in the command line with their contained args
    argv = []
    for a in sys.argv[1:]:
        if a.endswith('.qmt') and os.path.exists(a):
            argv.extend(getArgsFromQmt(a))
        else:
            argv.append(a)
    if argv != sys.argv[1:]:
        print '# Running', sys.argv[0], 'with arguments', argv

    opts, args = parser.parse_args(args=argv)

    # Check consistency of options

    # Parallel Option ---------------------------------------------------------
    if opts.ncpus:
        from multiprocessing import cpu_count
        sys_cpus = cpu_count()
        if opts.ncpus > sys_cpus:
            s = "Invalid value : --ncpus : only %i cpus available" % sys_cpus
            parser.error(s)
        elif opts.ncpus < -1:
            s = "Invalid value : --ncpus must be integer >= -1"
            parser.error(s)
    else:
        # FIXME: is it really needed to set it to None if it is 0 or False?
        opts.ncpus = None

    # configure the logging
    import logging
    from GaudiKernel.ProcessJobOptions import (InstallRootLoggingHandler,
                                               PrintOff)

    if opts.old_opts:
        prefix = "// "
    else:
        prefix = "# "
    level = logging.INFO
    if opts.debug:
        level = logging.DEBUG
    InstallRootLoggingHandler(prefix, level=level, with_time=opts.debug)
    root_logger = logging.getLogger()

    # tcmalloc support
    if opts.tcmalloc:
        opts.preload.insert(0, os.environ.get("TCMALLOCLIB", "libtcmalloc.so"))
    # allow preloading of libraries
    if opts.preload:
        preload = os.environ.get("LD_PRELOAD", "")
        if preload:
            preload = preload.replace(" ", ":").split(":")
        else:
            preload = []
        for libname in set(preload).intersection(opts.preload):
            logging.warning("Ignoring preload of library %s because it is "
                            "already in LD_PRELOAD.", libname)
        to_load = [libname
                   for libname in opts.preload
                   if libname not in set(preload)]
        if to_load:
            preload += to_load
            preload = ":".join(preload)
            os.environ["LD_PRELOAD"] = preload
            logging.info("Restarting with LD_PRELOAD='%s'", preload)
            # remove the --tcmalloc option from the arguments
            # FIXME: the --preload arguments will issue a warning but it's tricky to remove them
            args = [a for a in sys.argv if a !=
                    '-T' and not '--tcmalloc'.startswith(a)]
            os.execv(sys.executable, [sys.executable] + args)

    # Profiler Support ------
    if opts.profilerName:
        profilerName = opts.profilerName
        profilerExecName = ""
        profilerOutput = opts.profilerOutput or (profilerName + ".output")

        # To restart the application removing the igprof option and prepending the string
        args = getArgsWithoutoProfilerInfo(sys.argv)

        igprofPerfOptions = "-d -pp -z -o igprof.pp.gz".split()

        profilerOptions = ""
        if profilerName == "igprof":
            if not opts.profilerOutput:
                profilerOutput += ".profile.gz"
            profilerOptions = "-d -z -o %s" % profilerOutput
            profilerExecName = "igprof"

        elif profilerName == "igprofPerf":
            if not opts.profilerOutput:
                profilerOutput += ".pp.gz"
            profilerOptions = "-d -pp -z -o %s" % profilerOutput
            profilerExecName = "igprof"

        elif profilerName == "igprofMem":
            if not opts.profilerOutput:
                profilerOutput += ".mp.gz"
            profilerOptions = "-d -mp -z -o %s" % profilerOutput
            profilerExecName = "igprof"

        elif "valgrind" in profilerName:
            # extract the tool
            if not opts.profilerOutput:
                profilerOutput += ".log"
            toolname = profilerName.replace('valgrind', '')
            outoption = "--log-file"
            if toolname in ("massif", "callgrind", "cachegrind"):
                outoption = "--%s-out-file" % toolname
            profilerOptions = "--tool=%s %s=%s" % (
                toolname, outoption, profilerOutput)
            profilerExecName = "valgrind"

        elif profilerName == "jemalloc":
            opts.preload.insert(0, os.environ.get(
                "JEMALLOCLIB", "libjemalloc.so"))
            os.environ['MALLOC_CONF'] = "prof:true,prof_leak:true"
        else:
            root_logger.warning("Profiler %s not recognized!" % profilerName)

        # Add potential extra options
        if opts.profilerExtraOptions != "":
            profilerExtraOptions = opts.profilerExtraOptions
            profilerExtraOptions = profilerExtraOptions.replace("__", "--")
            profilerOptions += " %s" % profilerExtraOptions

        # now we look for the full path of the profiler: is it really there?
        if profilerExecName:
            import distutils.spawn
            profilerPath = distutils.spawn.find_executable(profilerExecName)
            if not profilerPath:
                root_logger.error("Cannot locate profiler %s" %
                                  profilerExecName)
                sys.exit(1)

        root_logger.info("------ Profiling options are on ------ \n"
                         " o Profiler: %s\n"
                         " o Options: '%s'.\n"
                         " o Output: %s" % (profilerExecName or profilerName, profilerOptions, profilerOutput))

        # allow preloading of libraries
        # That code need to be acsracted from above
        to_reload = []
        if opts.preload:
            to_reload = setLibraryPreload(opts.preload)

        if profilerExecName:
            # We profile python
            profilerOptions += " python"

            # now we have all the ingredients to prepare our command
            arglist = [profilerPath] + profilerOptions.split() + args
            arglist = [a for a in arglist if a != '']
            # print profilerPath
            # for arg in arglist:
            # print arg
            os.execv(profilerPath, arglist)
        else:
            arglist = [a for a in sys.argv if not a.startswith("--profiler")]
            os.execv(sys.executable, [sys.executable] + arglist)

    # End Profiler Support ------

    if opts.pickle_output:
        if opts.output:
            root_logger.error(
                "Conflicting options: use only --pickle-output or --output")
            sys.exit(1)
        else:
            root_logger.warning(
                "--pickle-output is deprecated, use --output instead")
            opts.output = opts.pickle_output

    from Gaudi.Main import gaudimain
    c = gaudimain()

    # Prepare the "configuration script" to parse (like this it is easier than
    # having a list with files and python commands, with an if statements that
    # decides to do importOptions or exec)
    options = ["importOptions(%r)" % f for f in args]
    # The option lines are inserted into the list of commands using their
    # position on the command line
    optlines = list(opts.options)
    # this allows to avoid to have to care about corrections of the positions
    optlines.reverse()
    for pos, l in optlines:
        options.insert(pos, l)

    # prevent the usage of GaudiPython
    class FakeModule(object):
        def __init__(self, exception):
            self.exception = exception

        def __getattr__(self, *args, **kwargs):
            raise self.exception
    sys.modules["GaudiPython"] = FakeModule(
        RuntimeError("GaudiPython cannot be used in option files"))

    # when the special env GAUDI_TEMP_OPTS_FILE is set, it overrides any
    # option(file) on the command line
    if 'GAUDI_TEMP_OPTS_FILE' in os.environ:
        options = ['importOptions(%r)' % os.environ['GAUDI_TEMP_OPTS_FILE']]
        PrintOff(100)

    # "execute" the configuration script generated (if any)
    if options:
        g = {}
        l = {}
        exec "from Gaudi.Configuration import *" in g, l
        for o in options:
            logging.debug(o)
            exec o in g, l

    import GaudiKernel.Proxy.Configurable
    if opts.no_conf_user_apply:
        logging.info("Disabling automatic apply of ConfigurableUser")
        # pretend that they have been already applied
        GaudiKernel.Proxy.Configurable._appliedConfigurableUsers_ = True

    # This need to be done before dumping
    if opts.old_conf_user_apply:
        from GaudiKernel.Proxy.Configurable import applyConfigurableUsers_old as applyConfigurableUsers
    else:
        from GaudiKernel.Proxy.Configurable import applyConfigurableUsers
    applyConfigurableUsers()

    # Options to be processed after applyConfigurableUsers
    if opts.post_options:
        g = {}
        l = {}
        exec "from Gaudi.Configuration import *" in g, l
        for o in opts.post_options:
            logging.debug(o)
            exec o in g, l

    if 'GAUDI_TEMP_OPTS_FILE' in os.environ:
        os.remove(os.environ['GAUDI_TEMP_OPTS_FILE'])
        opts.use_temp_opts = False

    if opts.verbose and not opts.use_temp_opts:
        c.printconfig(opts.old_opts, opts.all_opts)
    if opts.output:
        c.writeconfig(opts.output, opts.all_opts)

    if opts.use_temp_opts:
        fd, tmpfile = mkstemp('.opts')
        os.close(fd)
        c.writeconfig(tmpfile, opts.all_opts)
        os.environ['GAUDI_TEMP_OPTS_FILE'] = tmpfile
        logging.info('Restarting from pre-parsed options')
        os.execv(sys.executable, [sys.executable] + sys.argv)

    c.printsequence = opts.printsequence
    if opts.printsequence:
        if opts.ncpus:
            logging.warning(
                "--printsequence not supported with --ncpus: ignored")
        elif opts.dry_run:
            logging.warning(
                "--printsequence not supported with --dry-run: ignored")

    # re-enable the GaudiPython module
    del sys.modules["GaudiPython"]

    if not opts.dry_run:
        # Do the real processing
        retcode = c.run(opts.gdb, opts.ncpus)

        # Now saving the run information pid, retcode and executable path to
        # a file is requested
        if opts.run_info_file:
            import os
            import json
            run_info = {}
            run_info["pid"] = os.getpid()
            run_info["retcode"] = retcode
            if os.path.exists('/proc/self/exe'):
                # These options can be used only on unix platforms
                run_info["exe"] = os.readlink('/proc/self/exe')

            logging.info("Saving run info to: %s" % opts.run_info_file)
            with open(opts.run_info_file, "w") as f:
                json.dump(run_info, f)

        sys.exit(retcode)

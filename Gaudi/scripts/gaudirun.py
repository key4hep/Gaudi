#!/usr/bin/env python

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
    args = list(args) # make a temp copy
    while args:
        o = args.pop(0)
        if o.startswith('--profile'):
            if '=' not in o:
                args.pop(0)
        else:
            newargs.append(o)
    return newargs


#---------------------------------------------------------------------
if __name__ == "__main__":
    import os, sys
    from optparse import OptionParser
    parser = OptionParser(usage = "%prog [options] <opts_file> ...")
    parser.add_option("-n","--dry-run", action="store_true",
                      help="do not run the application, just parse option files")
    parser.add_option("-p","--pickle-output", action="store", type="string",
                      metavar = "FILE",
                      help="DEPRECATED: use '--output file.pkl' instead. Write "
                           "the parsed options as a pickle file (static option "
                           "file)")
    parser.add_option("-v","--verbose", action="store_true",
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
                      type = "string", nargs = 1,
                      help="add a single line (Python) option to the configuration. "
                           "All options lines are executed, one after the other, in "
                           "the same context.")
    parser.add_option("--no-conf-user-apply", action="store_true",
                      help="disable the automatic application of configurable "
                           "users (for backward compatibility)")
    parser.add_option("-o", "--output", action = "store", type = "string",
                      help ="dump the configuration to a file. The format of "
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

    parser.set_defaults(options = [],
                        tcmalloc = False,
                        profilerName = '',
                        profilerOutput = '',
                        profilerExtraOptions = '',
                        preload = [],
                        ncpus = None)

    opts, args = parser.parse_args()

    # Check consistency of options

    # Parallel Option ---------------------------------------------------------
    if opts.ncpus:
        from multiprocessing import cpu_count
        sys_cpus = cpu_count()
        if opts.ncpus > sys_cpus:
            s = "Invalid value : --ncpus : only %i cpus available" % sys_cpus
            parser.error(s)
        elif opts.ncpus < -1 :
            s = "Invalid value : --ncpus must be integer >= -1"
            parser.error(s)
    else:
        # FIXME: is it really needed to set it to None if it is 0 or False?
        opts.ncpus = None

    # configure the logging
    import logging
    from GaudiKernel.ProcessJobOptions import InstallRootLoggingHandler

    if opts.old_opts: prefix = "// "
    else: prefix = "# "
    level = logging.INFO
    if opts.debug:
        level = logging.DEBUG
    InstallRootLoggingHandler(prefix, level = level)
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
            args = [ a for a in sys.argv if a != '-T' and not '--tcmalloc'.startswith(a) ]
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
            toolname = profilerName.replace('valgrind','')
            outoption = "--log-file"
            if toolname in ("massif", "callgrind", "cachegrind"):
                outoption = "--%s-out-file" % toolname
            profilerOptions = "--tool=%s %s=%s" % (toolname, outoption, profilerOutput)
            profilerExecName = "valgrind"

        else:
            root_logger.warning("Profiler %s not recognized!" % profilerName)

        # Add potential extra options
        if opts.profilerExtraOptions!="":
            profilerExtraOptions = opts.profilerExtraOptions
            profilerExtraOptions = profilerExtraOptions.replace("__","--")
            profilerOptions += " %s" % profilerExtraOptions

        # now we look for the full path of the profiler: is it really there?
        import distutils.spawn
        profilerPath = distutils.spawn.find_executable(profilerExecName)
        if not profilerPath:
            root_logger.error("Cannot locate profiler %s" % profilerExecName)
            sys.exit(1)

        root_logger.info("------ Profiling options are on ------ \n"\
                         " o Profiler: %s\n"\
                         " o Options: '%s'.\n"\
                         " o Output: %s" % (profilerExecName, profilerOptions, profilerOutput))

        # We profile python
        profilerOptions += " python"

        # now we have all the ingredients to prepare our command
        arglist = [profilerPath] + profilerOptions.split() + args
        arglist = [ a for a in arglist if a!='' ]
        #print profilerPath
        #for arg in arglist:
            #print arg
        os.execv(profilerPath, arglist)

    # End Profiler Support ------

    if opts.pickle_output:
        if opts.output:
            root_logger.error("Conflicting options: use only --pickle-output or --output")
            sys.exit(1)
        else:
            root_logger.warning("--pickle-output is deprecated, use --output instead")
            opts.output = opts.pickle_output

    from Gaudi.Main import gaudimain
    c = gaudimain()

    # Prepare the "configuration script" to parse (like this it is easier than
    # having a list with files and python commands, with an if statements that
    # decides to do importOptions or exec)
    options = [ "importOptions(%r)" % f for f in args ]
    # The option lines are inserted into the list of commands using their
    # position on the command line
    optlines = list(opts.options)
    optlines.reverse() # this allows to avoid to have to care about corrections of the positions
    for pos, l in optlines:
        options.insert(pos,l)

    # prevent the usage of GaudiPython
    class FakeModule(object):
        def __init__(self, exception):
            self.exception = exception
        def __getattr__(self, *args, **kwargs):
            raise self.exception
    sys.modules["GaudiPython"] = FakeModule(RuntimeError("GaudiPython cannot be used in option files"))

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

    if opts.verbose:
        c.printconfig(opts.old_opts, opts.all_opts)
    if opts.output:
        c.writeconfig(opts.output, opts.all_opts)

    c.printsequence = opts.printsequence
    if opts.printsequence:
        if opts.ncpus:
            logging.warning("--printsequence not supported with --ncpus: ignored")
        elif opts.dry_run:
            logging.warning("--printsequence not supported with --dry-run: ignored")

    # re-enable the GaudiPython module
    del sys.modules["GaudiPython"]

    if not opts.dry_run:
        # Do the real processing
        sys.exit(c.run(opts.ncpus))

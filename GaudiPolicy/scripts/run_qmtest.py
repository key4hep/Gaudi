#!/usr/bin/env python
#
# Small wrapper script to simplify the execution of QMTest tests.
#
# @author: Marco Clemencic <marco.clemencic@cern.ch>
# @date: 22/10/2007
#
import os, sys

class Options(object):
    pass

def parseOptions(argv = None):
    if argv is None:
        argv = sys.argv[1:]
    # Defaults
    opts = Options()
    opts.package = "Unknown"
    opts.qmtest_args = []
    opts.have_user_options = False
    opts.output = os.path.normpath(os.path.expandvars(os.environ["QMTESTRESULTS"]))
    opts.qmtest_dir = os.path.normpath(os.path.expandvars(os.environ["QMTESTLOCALDIR"]))
    opts.dry_run = False
    if "GAUDI_QMTEST_HTML_OUTPUT" in os.environ:
        opts.html_output = os.path.normpath(os.path.expandvars(os.environ.get("GAUDI_QMTEST_HTML_OUTPUT")))
    else:
        opts.html_output = None

    # First argument is the package name:
    if argv:
        opts.package = argv.pop(0)

    # If the use specifies a directory where to collect all the results
    # (e.g. because running from a read-only location) we must use it
    if "QMTESTRESULTSDIR" in os.environ:
        opts.output = os.path.normpath(os.path.expandvars(os.environ["QMTESTRESULTSDIR"]))
        opts.output = os.path.join(opts.output,
                                   "%s.%s.qmr" % (opts.package, os.environ.get("CMTCONFIG", "noConfig")))

    # Do we have user options?
    opts.have_user_options = len(argv)
    # Scan the user options (if any) to look for options we must intercept
    while argv:
        o = argv.pop(0)
        if o in ['-o','--output']:
            # make the path absolute
            opts.output = os.path.realpath(argv.pop(0))
            opts.have_user_options -= 2
        elif o in ["--no-output"]:
            opts.output = None
            opts.have_user_options -= 1
        elif o in ["--dry-run"]:
            opts.dry_run = True
            opts.have_user_options -= 1
        elif o in ["--html-output"]:
            opts.html_output = os.path.realpath(argv.pop(0))
            opts.have_user_options -= 2
        else:
            opts.qmtest_args.append(o)
    # Add the option for the output to the qmtest_args
    if opts.output:
        opts.qmtest_args = ["-o", opts.output] + opts.qmtest_args
    else:
        opts.qmtest_args.insert(0, "--no-output")
    return opts

def main(argv = None):
    opts = parseOptions(argv)

    print "==========> Running tests for package %s" % opts.package

    # create the destination directory if necessary
    if opts.output:
        results_dest_dir = os.path.realpath(os.path.join(opts.qmtest_dir, os.path.dirname(opts.output)))
        if not os.path.exists(results_dest_dir):
            print "==========> Creating '%s'" % results_dest_dir
            os.makedirs(results_dest_dir, 0755)

    print "==========> Entering '%s'" % opts.qmtest_dir
    os.chdir(opts.qmtest_dir)

    qmtest_cmd = ["qmtest"]

    if not os.path.isdir("QMTest"):
        # If the test-db directory is not initialized, we have to initialize it
        try:
            os.mkdir("QMTest")
        except OSError:
            # we cannot create the directory... we need a temporary test database
            from tempfile import mkdtemp
            from shutil import copytree
            # the extra level is needed because of a limitation in copytree
            tdb = os.path.join(mkdtemp("GaudiTest"), "qmtest")
            copytree(".", tdb)
            os.chmod(tdb, 0700) # This is needed because copytree will copy also the restrictive permissions
            qmtest_cmd += ["--tdb", tdb]
        print "==========> Initializing QMTest database"
        os.system(" ".join(qmtest_cmd + ["create-tdb"]))

    if opts.html_output:
        opts.qmtest_args.insert(0, '''--result-stream "GaudiTest.HTMLResultStream(dir='%s')"'''
                                  % opts.html_output.replace("\\","\\\\"))

    # prepare the qmtest command
    cmd = " ".join(qmtest_cmd + ["run"] + opts.qmtest_args)

    if not opts.have_user_options:
        # If there are no user options, we should check for a default test suite
        # to run.
        # The suite to run is the first one available in the list passed via
        # the environment variable GAUDI_QMTEST_DEFAULT_SUITE (comma-separated list)
        # or the suite with the same name as the package.
        # If none of them is present, no argument is passed to qmtest, so all the tests
        # are run.
        suites = []
        if "GAUDI_QMTEST_DEFAULT_SUITE" in os.environ:
            suites.extend([s.strip().lower()
                           for s in os.environ["GAUDI_QMTEST_DEFAULT_SUITE"].split(",")])
        suites.append(opts.package.lower())
        for s in suites:
            if os.path.exists("%s.qms" % s):
                cmd += " %s" % s
                break

    if opts.dry_run:
        print "==========> Would run '%s'"%cmd
    else:
        print "==========> Running '%s'"%cmd
        os.system(cmd)

    # drop the temporary test DB if needed
    if "--tdb" in qmtest_cmd:
        from shutil import rmtree
        def onerror(func, path, exc_info):
            """
            Change the permissions and try again to remove a file or a dir.
            Ignore failures.
            """
            if func in [os.remove, os.rmdir]:
                # change permissions and try again
                try:
                    os.chmod(path, 0600)
                    os.chmod(os.path.dirname(path), 0700)
                    func(path)
                except os.error:
                    pass
        rmtree(os.path.dirname(qmtest_cmd[-1]), onerror = onerror)

# Note: the return code of qmtest is not propagated to avoid that
#       CMT stops if we have a non-PASS tests (e.g. UNTESTED).
if __name__ == '__main__':
    main()

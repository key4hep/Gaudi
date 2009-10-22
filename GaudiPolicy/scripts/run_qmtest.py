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
    # First argument is the package name:
    if argv:
        opts.package = argv.pop(0)
    
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

    if not os.path.isdir("QMTest"):
        # The QMTest database is not initialized
        print "==========> Initializing QMTest database"
        os.system("qmtest create-tdb")

    # prepare the qmtest command
    cmd = "qmtest run %s" % (" ".join(opts.qmtest_args))
    
    # check if we have a test suite called as the package
    if not opts.have_user_options and os.path.exists("%s.qms" % opts.package.lower()):
        cmd += " %s" % opts.package.lower()
    
    if opts.dry_run:
        print "==========> Would run '%s'"%cmd
    else:
        print "==========> Running '%s'"%cmd
        os.system(cmd)

# Note: the return code of qmtest is not propagated to avoid that
#       CMT stops if we have a non-PASS tests (e.g. UNTESTED).
if __name__ == '__main__':
    main()

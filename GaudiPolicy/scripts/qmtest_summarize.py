#!/usr/bin/env python
#
# Simple script to loop over the packages providing tests and collect
# the result summaries
#
# @author: Marco Clemencic <marco.clemencic@cern.ch>
# @date: 3/11/2007
#
import os, sys, re

def parse_summary(output):
    report = { "results":[], "not_passed":[], "statistics": {} }
    current_block = None
    for l in output.splitlines():
        if l.startswith("--- TEST RESULTS"):
            current_block = "results"
        elif l.startswith("--- TESTS THAT DID NOT PASS"):
            current_block = "not_passed"
        elif l.startswith("--- STATISTICS"):
            current_block = "statistics"
        else:
            if current_block in ["results","not_passed"]:
                report[current_block].append(l)
            elif current_block == "statistics":
                tkns = l.split()
                if tkns:
                    report["statistics"][tkns[-1]] = int(tkns[0])

    report["results"] = report["results"][:-1]
    report["not_passed"] = report["not_passed"][:-2]
    if report["not_passed"][-1].strip() == "None.":
        report["not_passed"] = []
    return report

def find_results_cmt():
    # I assume that this script is run from a CMT action, which implies few other
    # things:
    #   - we are in the cmt directory of a package
    #   - CMTCONFIG etc. are correctly defined
    #   - we can call cmt

    # If the use specifies a directory where to collect all the results
    # (e.g. because running from a read-only location) we must use it
    if "QMTESTRESULTSDIR" in os.environ:
        outputdir = os.path.normpath(os.path.expandvars(os.environ["QMTESTRESULTSDIR"]))
    else:
        outputdir = None

    ## Directory where to store the report
    #if "CMTINSTALLAREA" not in os.environ:
    #    os.environ["CMTINSTALLAREA"] = os.popen("cmt show macro_value CMTINSTALLAREA","r").read().strip()

    # Find the packages (their cmt dirs)
    cmt_br_header_re = re.compile(r"^# Now trying.*in (.*) \([0-9]+/[0-9]+\)")
    dirs = []

    # Hack to work around a problem with cmt broadcast and CMTUSERCONTEXT.
    old_CMTUSERCONTEXT = None
    if "CMTUSERCONTEXT" in os.environ:
        old_CMTUSERCONTEXT = os.environ["CMTUSERCONTEXT"]
        del os.environ["CMTUSERCONTEXT"]

    for l in os.popen("cmt broadcast","r"):
        m = cmt_br_header_re.match(l)
        if m:
            dirs.append(m.group(1))

    # Restore original CMTUSERCONTEXT, if it was set.
    if old_CMTUSERCONTEXT is not None:
        os.environ["CMTUSERCONTEXT"] = old_CMTUSERCONTEXT

    # Find the results.qmr files
    if not outputdir:
        results = filter(os.path.isfile,[ os.path.realpath(os.path.join(d,'..',os.environ["CMTCONFIG"],"results.qmr")) for d in dirs ])
    else:
        results = []
        for d in dirs:
            pkg = os.path.basename(os.path.dirname(d))
            r = os.path.join(outputdir, "%s.%s.qmr" % (pkg, os.environ.get("CMTCONFIG", "noConfig")))
            if os.path.isfile(r):
                results.append(r)

    return results

def find_results_cmake():
    results = []
    for r, ds, fs in os.walk(os.getcwd()):
        for f in fs:
            if f.endswith('.qmr'):
                results.append(os.path.join(r, f))
    return results

def find_results():
    if os.path.exists('CMakeCache.txt'):
        return find_results_cmake()
    else:
        return find_results_cmt()

def main():
    results = find_results()

    # Check if the result files have been found
    if not results:
        print "Warning: no result file found! (Did you run the tests?)"
        sys.exit()

    # parse and collect the summaries
    # FIXME: (MCl) with QMTest 2.3 we need a test database to be able to run "qmtest summarize", while it
    #              is not needed with QMTest 2.4
    from tempfile import mkdtemp
    from shutil import rmtree
    tmpdir = mkdtemp()
    origdir = os.getcwd()
    report = None
    try:
        os.chdir(tmpdir)
        os.popen("qmtest create-tdb","r").read()

        for r in results:
            out = os.popen("qmtest summarize -f brief %s"%r,"r").read()
            rep = parse_summary(out)
            if report is None:
                report = rep
            else:
                report["results"] += rep["results"]
                if rep["not_passed"]:
                    report["not_passed"] += rep["not_passed"]
                for k in rep["statistics"]:
                    if k in report["statistics"]:
                        report["statistics"][k] += rep["statistics"][k]
                    else:
                        report["statistics"][k] = rep["statistics"][k]
    finally:
        os.chdir(origdir)
        rmtree(tmpdir,ignore_errors=True)

    if report is None:
        print "Warning: I could not generate the report"
        sys.exit()

    # Finalize the report
    report["results"].append('')
    if not report["not_passed"]:
        report["not_passed"] = ['','  None.']
    report["not_passed"] += ['','']

    statistics_output = ['--- STATISTICS ---------------------------------------------------------------','']
    # order: total, FAIL, UNTESTED, PASS
    #"       2        tests total"
    tot = report["statistics"]["total"]
    statistics_output.append("%8d        tests total"%(tot))
    success = True
    for k in [ "ERROR", "FAIL", "UNTESTED", "PASS" ]:
        if k in report["statistics"]:
            n = report["statistics"][k]
            p = round(100. * n / tot)
            statistics_output.append("%8d (%3d%%) tests %s"%(n,p,k))
            if k in ["ERROR", "FAIL"]:
                success = False
    statistics_output.append('')

    results_output = ['--- TEST RESULTS -------------------------------------------------------------']
    results_output += report["results"]

    not_passed_output = ['--- TESTS THAT DID NOT PASS --------------------------------------------------']
    if not report["not_passed"]:
        not_passed_output
    not_passed_output += report["not_passed"]

    output = statistics_output + not_passed_output + results_output + not_passed_output + statistics_output
    print '\n'.join(output)
    return success


if __name__ == '__main__':
    if not main():
        # failure
        sys.exit(1)

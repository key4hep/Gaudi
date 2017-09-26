#! /usr/bin/env python

"""
This script acts as a wrapper in order to generate Gaudi configs for running 
with different parameters in order to perform performance measurements.
It takes command line arguments.
"""

import optparse
"""
NumberOfEvents = 10
NumberOfEventsInFlight = 5
NumberOfAlgosInFlight = 5
NumberOfThreads = 10
CloneAlgos = False
DumpQueues = False
"""


def createParser():
    usage = "%prog [options]"
    parser = optparse.OptionParser(usage)
    parser.add_option('-n', help='Number of events', dest='nevts', default=15)
    parser.add_option('--eif', help='Events in flight', dest='eif', default=5)
    parser.add_option('--aif', help='Algos in flight', dest='aif', default=10)
    parser.add_option('--nthreads', help='Number of threads',
                      dest='nthreads', default=10)
    parser.add_option('--clone', help='Clone Algos',
                      dest='clone', action='store_true', default=False)
    parser.add_option('--dumpQueues', help='Dump Queues',
                      dest='dumpqueues', action='store_true', default=False)
    parser.add_option('-v', help='Verbosity level',
                      dest='verbosity', default=5)
    parser.add_option('--exec', help='ExecuteWorkflow',
                      dest='execbrunel', action='store_true', default=False)
    parser.add_option('--bg', help='Launch in background',
                      dest='bg', action='store_true', default=False)
    parser.add_option('--scale', help='Scale Algorithms time',
                      dest='scale', default=1)
    options, args = parser.parse_args()
    return options


def replaceValues(cfg_name, n, eif, aif, nthreads, scale, clone, dumpQueues, verbosity):

    newcfglines = open(cfg_name, "r").readlines()
    cfg_name = cfg_name.replace(".py", "")
    verb = ""
    if verbosity != 6:
        verb = "_v%s" % verbosity
    scale_s = ""
    if scale != 1:
        scale_s = "_s%s" % scale
    newcfgname = "measurement_%s_n%s_eif%s_aif%s_nthreads%s_c%s_dq%s%s%s.py" % (
        cfg_name, n, eif, aif, nthreads, clone, dumpQueues, scale_s, verb)
    newcfg = open(newcfgname, "w")
    for line in newcfglines:
        if ('NumberOfEvents' in line and 'NUMBEROFEVENTS' in line and not "FLIGHT" in line):
            line = line.replace("NUMBEROFEVENTS", str(n))
        if ('NumberOfEventsInFlight' in line and 'NUMBEROFEVENTSINFLIGHT' in line):
            line = line.replace("NUMBEROFEVENTSINFLIGHT", str(eif))
        if ('NumberOfAlgosInFlight' in line and 'NUMBEROFALGOSINFLIGHT' in line):
            line = line.replace("NUMBEROFALGOSINFLIGHT", str(aif))
        if ('NumberOfThreads' in line and 'NUMBEROFTHREADS' in line):
            line = line.replace("NUMBEROFTHREADS", str(nthreads))
        if ('DumpQueues'in line and 'DUMPQUEUES' in line):
            line = line.replace("DUMPQUEUES", str(dumpQueues))
        if ('CloneAlgos'in line and 'CLONEALGOS' in line):
            line = line.replace("CLONEALGOS", str(clone))
        if ('Verbosity'in line and 'VERBOSITY' in line):
            line = line.replace("VERBOSITY", str(verbosity))
        if ('Scale'in line and 'SCALE' in line):
            line = line.replace("SCALE", str(scale))
        newcfg.write(line)
    newcfg.close()
    return newcfgname


if __name__ == "__main__":
    options = createParser()
    newcfg = replaceValues("BrunelScenario.py",
                           options.nevts,
                           options.eif,
                           options.aif,
                           options.nthreads,
                           float(options.scale),
                           options.clone,
                           options.dumpqueues,
                           options.verbosity)

    logfile = newcfg.replace(".py", ".log")
    gaudirun = "`alias gaudirun`"
    gaudirun = "/afs/cern.ch/user/d/dpiparo/Gaudi/build.x86_64-slc5-gcc46-opt/run gaudirun.py"
    command = "/usr/bin/time -f %%S -o timing_%s %s %s >& %s  " % (
        logfile, gaudirun, newcfg, logfile)
    if options.bg:
        command += " &"

    print command
    if options.execbrunel:
        import os
        os.system(command)

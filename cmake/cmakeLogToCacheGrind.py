#!/usr/bin/env python
'''
This script allows to parse a log file coming from the instrumentation of the
Gaudi cmake process (See Instrument.cmake) and create an other file
understandable by kcachegrind for easy visualization of where we spend time in
cmake.
'''
import sys


def usage():
    print "Invalid arguments\nProper syntax is :\n  %s <log file> <callgrind file>" % sys.argv[
        0]


if len(sys.argv) != 3:
    usage()
    sys.exit(-1)

# dictionary storinf function calls
# actuall a dictionnary of dictionnaries, with top level key
# storing caller name, second level key storing callee name
# and values storing a tuple (nb calls, total time spent in calls)
calls = {'main': {}}

# dictionnary of total time spent in functions
timeSpentInfunc = {}

# currrent call stack
# used for transforming the list of log entries into a graph of calls
callStack = []

# sum of total time spent
totalcost = 0

# Here we go through the log lines starting with 'TIMING'
# and get time info. We then update the calls and timeSpentInfunc
# dictionnaries
for line in open(sys.argv[1]).readlines():
    if not line.startswith('TIMING'):
        continue
    key, func, time = line.split()[2:]
    if key == 'STARTIME':
        callStack.append((func, int(time)))
    elif key == 'ENDTIME':
        sfunc, stime = callStack.pop()
        if sfunc != func:
            print 'Mismatch START/END for %s/%s' % (sfunc, func)
            sys.exit()
        deltatime = int(time) - stime
        # add time spent to this function
        if func not in timeSpentInfunc:
            timeSpentInfunc[func] = 0
        timeSpentInfunc[func] += deltatime
        # count callbacks and time spent in upper function
        if callStack:
            caller = callStack[-1][0]
        else:
            caller = 'main'
            # totalcost
            totalcost += deltatime
        if caller not in calls:
            calls[caller] = {}
        if func not in calls[caller]:
            calls[caller][func] = (0, 0)
        nbCalls, timeSpent = calls[caller][func]
        calls[caller][func] = (nbCalls + 1, timeSpent + deltatime)


def writeCalls(func, callees):
    '''writes the callgrind entry for a given function given its
    name and a dictionnary of callees -> (nb calls, time spent)'''
    output.write('fn=%s\n' % func)
    # compute time spent in this function
    # excluding time spend in callees
    if func in timeSpentInfunc:
        tottime = timeSpentInfunc[func]
        for cfunc in callees:
            tottime -= callees[cfunc][1]
        output.write('0 %d \n' % tottime)
    else:
        output.write('0 0 \n')
    # print callees and nb calls/time spent
    # for each of them
    for cfunc in callees:
        nb, time = callees[cfunc]
        output.write('cfn=%s\n' % cfunc)
        output.write('calls=%d 0\n' % nb)
        output.write('0 %d\n' % time)
    output.write('\n')


# create output callgrind file
output = open(sys.argv[2], 'w')
output.write('events: Cycles\n')
output.write('summary: %d\n' % totalcost)
output.write('fl=GaudiProjectConfig.cmake\n')
for func in calls:
    writeCalls(func, calls[func])
for func in timeSpentInfunc:
    if func not in calls:
        output.write('fn=%s\n' % func)
        output.write('0 %d\n' % timeSpentInfunc[func])
output.close()

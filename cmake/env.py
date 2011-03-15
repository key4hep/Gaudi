#!/usr/bin/env python
"""
Small script to execute a command in a modified environment (see man 1 env).
"""
import os
from optparse import OptionParser

parser = OptionParser(prog = "env",
                      usage = "Usage: %prog [OPTION]... [NAME=VALUE]... [COMMAND [ARG]...]",
                      description = "Set each NAME to VALUE in the environment and run COMMAND.",
                      epilog = "If no COMMAND, print the resulting environment.")
parser.add_option("-i", "--ignore-environment",
                  action = "store_true",
                  help = "start with an empty environment")
parser.add_option("-u", "--unset",
                  metavar = "NAME",
                  action = "append",
                  help = "remove variable from the environment")
parser.disable_interspersed_args()
parser.set_defaults(unset = [],
                    ignore_environment = False)

opts, args = parser.parse_args()

if opts.ignore_environment:
    env = {}
else:
    env = dict(os.environ)

for n in opts.unset:
    if n in env:
        print "unset", n
        del env[n]

while args and "=" in args[0]:
    n, v = args.pop(0).split("=", 1)
    v = v.replace('[:]', os.pathsep) # needed as a hack on CMake + win
    if n[-1] in ['>', '<']:
        action = n[-1]
        n = n[:-1]
    if n in env:
        if action == '<': # prepend
            v = v + os.pathsep + env[n]
        elif action == '>': # append
            v = env[n] + os.pathsep + v
    print "set %s=%s" % (n,v)
    env[n] = v

if not args:
    for nv in env.items():
        print "%s=%s" % nv
else:
    import sys
    from subprocess import Popen
    sys.exit(Popen(args, env = env).wait())

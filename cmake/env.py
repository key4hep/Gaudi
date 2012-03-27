#!/usr/bin/env python
"""
Small script to execute a command in a modified environment (see man 1 env).
"""
import os

def set_env(env, set = [], unset = [], append = [], prepend = []):
    """
    Manipulate the dictionary-like object 'env' according to the prescriptions in
    the lists 'unset', 'set', 'append' and 'prepend' (in this order).
    The lists must contain strings of the format 'NAME=VALUE' except for 'unset'
    which requires only 'NAME'.
    """
    def parse(x):
        """split the "NAME=VALUE" string into the tuple ("NAME", "VALUE")
        replacing '[:]' with os.pathsep in VALUE"""
        n, v = x.split('=', 1)
        return n, v.replace('[:]', os.pathsep)
    def dictlist(l):
        """create a dictionary from the list of pairs by appending to lists"""
        d = {}
        for n, v in l:
            if n in d:
                d[n].append(v)
            else:
                d[n] = [v]
        return d
    # remove the 'unset' variables
    for n in unset:
        if n in env:
            log.debug("unset %s", n)
            del env[n]
    # set the requested variables
    env.update(map(parse, set))
    # append
    for n, v in dictlist(map(parse, append)).items():
        if n in env:
            v.insert(0, env[n]) # add old value at the beginning
        env[n] = os.pathsep.join(v)
    # prepend
    for n, v in dictlist(map(parse, reversed(prepend))).items():
        if n in env:
            v.append(env[n]) # add old value at end
        env[n] = os.pathsep.join(v)

    return env


def main():
    from optparse import OptionParser
    parser = OptionParser(prog = "env",
                          usage = "Usage: %prog [OPTION]... [NAME=VALUE]... [COMMAND [ARG]...]",
                          description = "Set each NAME to VALUE in the environment and run COMMAND.",
                          epilog = "The operations are performed in the order: unset, set, append, "
                                   "prepend. If no COMMAND, print the resulting environment." )
    parser.add_option("-i", "--ignore-environment",
                      action = "store_true",
                      help = "start with an empty environment")
    parser.add_option("-u", "--unset",
                      metavar = "NAME",
                      action = "append",
                      help = "remove variable from the environment")
    parser.add_option("-s", "--set",
                      metavar = "NAME=VALUE",
                      action = "append",
                      help = "set the variable NAME to VALUE")
    parser.add_option("-a", "--append",
                      metavar = "NAME=VALUE",
                      action = "append",
                      help = "append VALUE to the variable NAME (with a '%s' as separator)" % os.pathsep)
    parser.add_option("-p", "--prepend",
                      metavar = "NAME=VALUE",
                      action = "append",
                      help = "prepend VALUE to the variable NAME (with a '%s' as separator)" % os.pathsep)
    parser.add_option("-x", "--xml",
                      action = "append",
                      help = "XML file describing the changes to the environment")
    parser.disable_interspersed_args()
    parser.set_defaults(unset = [],
                        set = [],
                        append = [],
                        prepend = [],
                        xml = [],
                        ignore_environment = False)

    opts, args = parser.parse_args()

    # find the 'set' arguments in the list of arguments
    i = 0
    argc = len(args)
    while (i < argc) and ("=" in args[i]):
        i += 1
    opts.set.extend(args[:i])
    cmd = args[i:]

    # prepare initial dictionary
    if opts.ignore_environment:
        env = {}
    else:
        env = dict(os.environ)

    env = set_env(env,
                  set = opts.set, unset = opts.unset,
                  append = opts.append, prepend = opts.prepend)

    if opts.xml:
        from EnvConfig import Control
        control = Control.Environment()
        # declare few known scalar variables
        for v in filter(lambda x: x in env, ["DISPLAY", "LS_COLORS", "GPG_AGENT_INFO",
                                             "KONSOLE_DBUS_SERVICE", "SESSION_MANAGER", "DBUS_SESSION_BUS_ADDRESS"]):
            control.declare(v, "scalar", False)
        for k in env:
            control.set(k, env[k])

        for f in opts.xml:
            control.loadXML(f)
        env = control.vars()

    if not cmd:
        for nv in env.items():
            print "%s=%s" % nv
        return 0
    else:
        from subprocess import Popen
        return Popen(cmd, env = env).wait()

if __name__ == "__main__":
    import sys
    sys.exit(main())

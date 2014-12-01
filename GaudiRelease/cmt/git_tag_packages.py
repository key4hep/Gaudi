#!/usr/bin/env python
"""
Small script to create the git tags for a release of Gaudi.
"""
__author__ = "Marco Clemencic <Marco.Clemencic@cern.ch>"

import os
import re
from subprocess import Popen, PIPE, STDOUT

def cmakelists(path):
    """
    Generator yielding the paths of the requirements files in a tree.
    """
    for dirpath, dirnames, filenames in os.walk(path):
        if 'CMakeLists.txt' in filenames and dirpath != path:
            dirnames[:] = [] # no need to recurse if we have a 'CMakeLists.txt'
            yield os.path.join(dirpath, 'CMakeLists.txt')
        else:
            # some directories should be ignored
            dirnames[:] = [dirname for dirname in dirnames
                           if not dirname.startswith('build.') and
                              dirname != 'cmake']

def getSubdirVer(cmakelists):
    """
    Extract subdir name and version from a CMakeLists.txt file.
    """
    pattern = re.compile(r"^\s*gaudi_subdir\s*\(\s*(\S+)\s+(\S+)\s*\)\s*$")
    for l in open(cmakelists):
        m = pattern.match(l.strip())
        if m:
            return m.groups()
    return (None, None)

def main():
    #from optparse import OptionParser
    root = os.path.realpath(__file__)
    root = os.path.dirname(root)
    root = os.path.dirname(root)
    root = os.path.dirname(root)
    for p, v in map(getSubdirVer, cmakelists(root)):
        if p and v:
            cmd = ["git", "tag", "%s/%s" % (p, v)]
            print " ".join(cmd) + ":",
            git = Popen(cmd, stdout=PIPE, stderr=PIPE)
            out, err = git.communicate()
            if git.returncode == 0:
                print "done"
            else:
                print "failed"
        elif p:
            print "WARNING: no version in subdir", p
        elif v:
            print "WARNING: found CMakeLists.txt with version (%s), but no subdir name" % v

if __name__ == "__main__":
    main()

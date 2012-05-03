#!/usr/bin/env python
"""
Small script to create the git tags for a release of Gaudi.
"""
__author__ = "Marco Clemencic <Marco.Clemencic@cern.ch>"

import os
import re
from subprocess import Popen, PIPE, STDOUT

def requirements(path):
    """
    Generator yielding the paths of the requirements files in a tree.
    """
    for root, dirs, files in os.walk(path):
        if "requirements" in files:
            dirs[:] = [] # no need to recurse if we have a 'requirements'
            yield os.path.join(root, "requirements")

def getPackVer(requirements):
    """
    Extract package name and package version from a requirements file.
    """
    pattern = re.compile(r"^\s*(version|package)\s*(\S+)\s*$")
    data = dict([m.groups()
                 for m in map(pattern.match, open(requirements))
                 if m])
    return data.get("package", None), data.get("version", None)
    
def main():
    #from optparse import OptionParser
    root = os.path.realpath(__file__)
    root = os.path.dirname(root)
    root = os.path.dirname(root)
    root = os.path.dirname(root)
    for p, v in map(getPackVer, requirements(root)):
        if p and v:
            cmd = ["git", "tag", "%s/%s" % (p, v)]
            print " ".join(cmd) + ":",
            git = Popen(cmd, stdout=PIPE, stderr=PIPE)
            out, err = git .communicate()
            if git.returncode == 0:
                print "done"
            else:
                print "failed"
        elif p:
            print "WARNING: no version in package", p
        elif v:
            print "WARNING: found requirements with version (%s), but no package" % v

if __name__ == "__main__":
    main()

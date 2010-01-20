#!/usr/bin/env python
"""
Small script finalize the "-pre" tags into regular tags for the packages in Gaudi.

See https://twiki.cern.ch/twiki/bin/view/Gaudi/GaudiSVNRepository for a
description of the repository structure.
"""
__author__ = "Marco Clemencic <Marco.Clemencic@cern.ch>"

import os, sys
import tempfile, shutil, re
from subprocess import Popen, PIPE

def _spawn(*args, **kwargs):
    cmd = args
    cwd = kwargs.get("cwd", os.getcwd())
    print ">>>> %s: %s" % (cwd, cmd)
    return apply(Popen, (cmd,), kwargs)
def _exec_cmd(*args, **kwargs):
    return apply(_spawn, args, kwargs).wait()
def _exec_cmd_output(*args, **kwargs):
    kwargs["stdout"] = PIPE
    p = apply(_spawn, args, kwargs)
    out = p.communicate()[0]
    return out, p.returncode

def svn(*args, **kwargs):
    return apply(_exec_cmd, ["svn"] + list(args), kwargs)

def svn_output(*args, **kwargs):
    return apply(_exec_cmd_output, ["svn"] + list(args), kwargs)

for c in ["co", "up", "ci", "ls", "add", "rm", "cp", "mkdir", "mv", "pg"]:
    exec """
def svn_%(cmd)s(*args, **kwargs):
    return apply(svn, ["%(cmd)s"] + list(args), kwargs)
def svn_%(cmd)s_output(*args, **kwargs):
    return apply(svn_output, ["%(cmd)s"] + list(args), kwargs)
""" % {"cmd": c}

svn_ci_real = svn_ci
def svn_ci_fake(args, cwd = "."):
    cmd = ["svn", "ci"] + args
    print "(n)> %s: %s" % (cwd, cmd)
#svn_ci = svn_ci_fake

def basename(url):
    return url.rsplit("/", 1)[-1]

def dirname(url):
    return url.rsplit("/", 1)[1]

def svn_exists(url):
    d,b = url.rsplit("/", 1)
    l = [x.rstrip("/") for x in svn_ls(d)]
    return b in l 

def checkout_structure(url, proj):
    def checkout_level(base):
        dirs = ["%s/%s" % (base, d) for d in svn_ls_output(base)[0].splitlines() if d.endswith("/")]
        apply(svn, ["up", "-N"] + dirs)
        return dirs
    root = basename(url)
    svn("co","-N", url, root)
    old_dir = os.getcwd()
    os.chdir(root)
    packages = [ pkg
                 for pkg, prj in
                   [ x.split()
                     for x in [ x.strip()
                                for x in svn_pg_output("packages", ".")[0].splitlines()]
                     if x and not x.startswith("#") ]
                 if prj == proj ]
    pkg_tag_dirs = [ proj + "/tags/" + pkg for pkg in packages ]    
    apply(svn, ["up", "-N", proj, proj + "/tags"] + pkg_tag_dirs)
    os.chdir(old_dir)
    return root, packages

def svn_listdir(path):
    return [f.rstrip("/") for f in svn_ls_output(path)[0].splitlines()]

def main():
    url = "svn+ssh://svn.cern.ch/reps/gaudi"
    proj = "Gaudi"
    tempdir = tempfile.mkdtemp()
    try:
        os.chdir(tempdir)
        
        # prepare repository structure (and move to its top level)
        root, packages = checkout_structure(url, proj)
        os.chdir(root)
        
        pre_rx = re.compile("(v[0-9]+r[0-9]+(p[0-9]+)?)-pre([0-9]*)")
        for p in packages:
            entries = svn_listdir("%s/tags/%s" %(proj, p))
            # get the list of -pre tags in the package 
            pre_tags = [ (t.group(0), t.group(1), t.group(3))
                         for t in map(pre_rx.match, entries)
                         if t ]
            # extract the latest -pre tag for each tag
            tags = {}
            for pt, t, ptv in pre_tags:
                if ptv:
                    ptv = int(ptv)
                else:
                    ptv = -1
                if (t not in tags) or (tags[t][0] < ptv):
                    tags[t] = (ptv, pt)
            # throw away unneeded information from the map
            tags = dict(zip(tags.keys(), map(lambda a: a[1], tags.values())))
            for tag in tags:
                pretagdir = "%s/tags/%s/%s" %(proj, p, tags[tag])
                tagdir = "%s/tags/%s/%s" %(proj, p, tag)
                if tag not in entries:
                    svn_up("-N", pretagdir)
                    svn_cp(pretagdir, tagdir)
            
        return svn_ci()
        
    finally:
        shutil.rmtree(tempdir, ignore_errors = True)
        pass

if __name__ == '__main__':
    sys.exit(main())

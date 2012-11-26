#!/usr/bin/env python
"""
Small script to prepare the tags and the distribution special directory for a
release of Gaudi.
See https://twiki.cern.ch/twiki/bin/view/Gaudi/GaudiSVNRepository for a
description of the repository structure.
"""
__author__ = "Marco Clemencic <Marco.Clemencic@cern.ch>"

import os, re, sys, tempfile, shutil
from subprocess import Popen, PIPE

_req_version_pattern = re.compile(r"^\s*version\s*(v[0-9]+r[0-9]+(?:p[0-9]+)?)\s*$")
def extract_version(f):
    """
    Find the version number in a requirements file.
    """
    global _req_version_pattern
    for l in open(f):
        m = _req_version_pattern.match(l)
        if m:
            return m.group(1)
    return None

_use_pattern = re.compile(r"^\s*use\s*(\w+)\s*(v[0-9]+r[0-9]+(?:p[0-9]+)?)\s*(\w+)?\s*$")
def gather_new_versions(f):
    global _use_pattern
    versions = {}
    for l in open(f):
        m = _use_pattern.match(l)
        if m:
            versions[m.group(1)] = m.group(2)
    return versions

def svn(*args, **kwargs):
    print "> svn", " ".join(args)
    return apply(Popen, (["svn"] + list(args),), kwargs)

def svn_ls(url):
    return svn("ls", url, stdout = PIPE).communicate()[0].splitlines()

def basename(url):
    return url.rsplit("/", 1)[-1]

def dirname(url):
    return url.rsplit("/", 1)[1]

def svn_exists(url):
    d,b = url.rsplit("/", 1)
    l = [x.rstrip("/") for x in svn_ls(d)]
    return b in l

def checkout_structure(url, proj, branch):
    def checkout_level(base):
        dirs = ["%s/%s" % (base, d) for d in svn_ls(base) if d.endswith("/")]
        apply(svn, ["up", "-N"] + dirs).wait()
        return dirs

    root = basename(url)
    svn("co","-N", url, root).wait()
    old_dir = os.getcwd()
    os.chdir(root)
    svn("up", "-N", proj).wait()
    br = [proj] + branch.split("/")
    for base in [ "/".join(br[:n+1]) for n in range(len(br))]:
        checkout_level(base)
    checkout_level(proj + "/tags")
    os.chdir(old_dir)
    return root

def main():
    from optparse import OptionParser
    parser = OptionParser()
    parser.add_option("--pre", action = "store_true",
                      help = "Create -pre tags instead of final tags.")
    parser.add_option("-b", "--branch",
                      help = "Use the given (global) branch as source for the tags instead of the trunk")
    opts, args = parser.parse_args()
    if opts.branch:
        opts.branch = "/".join(["branches", "GAUDI", opts.branch])
    else:
        opts.branch = "trunk"

    url = "svn+ssh://svn.cern.ch/reps/gaudi"
    proj = "Gaudi"
    container = "GaudiRelease"
    packages = gather_new_versions("requirements")
    packages[container] = extract_version("requirements")
    tempdir = tempfile.mkdtemp()
    try:
        os.chdir(tempdir)
        # prepare repository structure (and move to its top level)
        os.chdir(checkout_structure(url, proj, opts.branch))

        # note that the project does not have "-pre"
        pvers = "%s_%s" % (proj.upper(), packages[container])

        # prepare project tag
        ptagdir = "%s/tags/%s/%s" % (proj, proj.upper(), pvers)
        if not svn_exists(ptagdir):
            svn("mkdir", ptagdir).wait()
            for f in ["cmt", "Makefile.cmt",
                      "Makefile-cmake.mk", "cmake", "CMakeLists.txt",
                      "configure", "toolchain.cmake"]:
                svn("cp", "/".join([proj, opts.branch, f]), "/".join([ptagdir, f])).wait()

        # prepare package tags
        tag_re = re.compile(r"^v(\d+)r(\d+)(?:p(\d+))?$")
        for p in packages:
            tag = packages[p]
            pktagdir = "%s/tags/%s/%s" % (proj, p, tag)
            # I have to make the tag if it doesn't exist and (if we use -pre tags)
            # neither the -pre tag exists.
            no_tag = not svn_exists(pktagdir)
            make_tag = no_tag or (opts.pre and no_tag and not svn_exists(pktagdir + "-pre"))
            if make_tag:
                if opts.pre:
                    pktagdir += "-pre"
                svn("cp", "/".join([proj, opts.branch, p]), pktagdir).wait()
                # Atlas type of tag
                tagElements = tag_re.match(tag)
                if tagElements:
                    tagElements = "-".join([ "%02d" % int(el or "0") for el in tagElements.groups() ])
                    pktagdir = "%s/tags/%s/%s-%s" % (proj, p, p, tagElements)
                    svn("cp", "/".join([proj, opts.branch, p]), pktagdir).wait()
            else:
                if not no_tag:
                    svn("up", "-N", pktagdir).wait() # needed for the copy in the global tag

        if not opts.pre:
            # prepare the full global tag too
            for p in packages:
                tag = packages[p]
                pktagdir = "%s/tags/%s/%s" % (proj, p, tag)
                svn("cp", pktagdir, "%s/%s" % (ptagdir, p)).wait()

        svn("ci").wait()

    finally:
        shutil.rmtree(tempdir, ignore_errors = True)

    return 0

if __name__ == '__main__':
    sys.exit(main())

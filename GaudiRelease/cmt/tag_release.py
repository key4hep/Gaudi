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
    
def main():
    use_pre = len(sys.argv) > 1 and 'pre' in  sys.argv
    url = "svn+ssh://svn.cern.ch/reps/gaudi"
    puburl = "https://svnweb.cern.ch/guest/gaudi"
    packages = gather_new_versions("requirements")
    packages["GaudiRelease"] = extract_version("requirements")
    tempdir = tempfile.mkdtemp()
    try:
        os.chdir(tempdir)
        # prepare root dir
        svn("co","-N", url, "gaudi").wait()
        os.chdir("gaudi")
        
        pvers = packages["GaudiRelease"]
        if use_pre:
            pvers += "-pre"
        
        to_commit = []
        
        # prepare project tag
        if (pvers + "/") not in svn_ls("%s/projects/GAUDI/tags" % url):
            svn("co", "-N", "%s/projects/GAUDI/tags" % url, "ProjTags").wait()
            svn("cp", "%s/projects/GAUDI/trunk" % url, os.path.join("ProjTags",pvers)).wait()
            to_commit.append("ProjTags")
        
        # prepare package tags
        for p in packages:
            tagsdir = "%s/packages/%s/tags" % (url, p) 
            tags = svn_ls(tagsdir)
            tag = packages[p]
            pre_tag = tag + "-pre"
            # I have to make the tag if it doesn't exist and (if we use -pre tags)
            # neither the -pre tag exists.
            make_tag = (tag+"/") not in tags and (use_pre and (pre_tag+"/") not in tags)
            if make_tag:
                svn("co", "-N", tagsdir, "%s_Tags" % p).wait()
                if use_pre:
                    tag = pre_tag
                svn("cp", "%s/packages/%s/trunk" % (url, p), "%s_Tags/%s" % (p, tag)).wait()
                to_commit.append("%s_Tags" % p)
        
        # prepare "distribution" dir
        distvers = pvers
        if ("GAUDI_%s/" % distvers) not in svn_ls("%s/distribution/tags/GAUDI" % url):
            svn("co","-N", "%s/distribution/tags/GAUDI" % url, "DistTags").wait()
            svn("mkdir", os.path.join("DistTags", "GAUDI_%s" % distvers)).wait()
            externals = open(os.path.join(tempdir, "svn_externals"), "w")
            pks = packages.keys()
            pks.sort()
            for p in pks:
                externals.write("%(pack)-20s %(url)s/packages/%(pack)s/tags/%(vers)s\n" % { "pack": p,
                                                                                            "vers": packages[p],
                                                                                            "url" : puburl })
            externals.write("%(dir)-20s %(url)s/projects/%(proj)s/tags/%(vers)s\n" % { "dir" : "cmt",
                                                                                       "proj": "GAUDI",
                                                                                       "vers": pvers,
                                                                                       "url" : puburl })
            externals.close()
            svn("ps", "-F", externals.name, "svn:externals", os.path.join("DistTags", "GAUDI_%s" % distvers)).wait()
            to_commit.append("DistTags")
        
        if to_commit:
            apply(svn, ["ci"] + to_commit).wait()
        else:
            print "Nothing to commit"
        
    finally:
        shutil.rmtree(tempdir, ignore_errors = True)
    
    return 0

if __name__ == '__main__':
    sys.exit(main())

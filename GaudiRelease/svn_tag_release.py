#!/usr/bin/env python
"""
Small script to prepare the tags and the distribution special directory for a
release of Gaudi.
See https://twiki.cern.ch/twiki/bin/view/Gaudi/GaudiSVNRepository for a
description of the repository structure.
"""
__author__ = "Marco Clemencic <Marco.Clemencic@cern.ch>"

import os
import re
import sys
import tempfile
import shutil
from subprocess import Popen, PIPE
from ConfigParser import ConfigParser


def svn(*args, **kwargs):
    print "> svn", " ".join(args)
    return Popen(["svn"] + list(args), **kwargs)


def svn_ls(url):
    return svn("ls", url, stdout=PIPE).communicate()[0].splitlines()


def basename(url):
    return url.rsplit("/", 1)[-1]


def dirname(url):
    return url.rsplit("/", 1)[1]


def svn_exists(url):
    d, b = url.rsplit("/", 1)
    l = [x.rstrip("/") for x in svn_ls(d)]
    return b in l


def checkout_structure(url, proj, branch):
    def checkout_level(base):
        dirs = ["%s/%s" % (base, d) for d in svn_ls(base) if d.endswith("/")]
        svn("up", "-N", *args).wait()
        return dirs

    root = basename(url)
    svn("co", "-N", url, root).wait()
    old_dir = os.getcwd()
    os.chdir(root)
    svn("up", "-N", proj).wait()
    br = [proj] + branch.split("/")
    for base in ["/".join(br[:n + 1]) for n in range(len(br))]:
        checkout_level(base)
    checkout_level(proj + "/tags")
    os.chdir(old_dir)
    return root


def main():
    from optparse import OptionParser
    parser = OptionParser()
    parser.add_option(
        "--pre",
        action="store_true",
        help="Create -pre tags instead of final tags.")
    parser.add_option(
        "-b",
        "--branch",
        help=
        "Use the given (global) branch as source for the tags instead of the trunk"
    )
    opts, args = parser.parse_args()
    if opts.branch:
        opts.branch = "/".join(["branches", "GAUDI", opts.branch])
    else:
        opts.branch = "trunk"

    url = "svn+ssh://svn.cern.ch/reps/gaudi"
    proj = "Gaudi"
    container = "GaudiRelease"
    project_info = ConfigParser()
    project_info.optionxform = str  # make options case sensitive
    project_info.read('project.info')
    packages = dict(project_info.items('Packages'))
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
            svn('cp', '/'.join([proj, opts.branch]), ptagdir).wait()

        # prepare package tags
        tag_re = re.compile(r"^v(\d+)r(\d+)(?:p(\d+))?$")
        for p in packages:
            tag = packages[p]
            pktagdir = "%s/tags/%s/%s" % (proj, p, tag)
            # I have to make the tag if it doesn't exist and (if we use -pre tags)
            # neither the -pre tag exists.
            no_tag = not svn_exists(pktagdir)
            make_tag = no_tag or (opts.pre and no_tag
                                  and not svn_exists(pktagdir + "-pre"))
            if make_tag:
                if opts.pre:
                    pktagdir += "-pre"
                svn("cp", "/".join([proj, opts.branch, p]), pktagdir).wait()
                # Atlas type of tag
                tagElements = tag_re.match(tag)
                if tagElements:
                    tagElements = "-".join([
                        "%02d" % int(el or "0") for el in tagElements.groups()
                    ])
                    pktagdir = "%s/tags/%s/%s-%s" % (proj, p, p, tagElements)
                    svn("cp", "/".join([proj, opts.branch, p]),
                        pktagdir).wait()
            else:
                if not no_tag:
                    # needed for the copy in the global tag
                    svn("up", "--depth=empty", pktagdir).wait()

        svn("ci").wait()

    finally:
        shutil.rmtree(tempdir, ignore_errors=True)

    return 0


if __name__ == '__main__':
    sys.exit(main())

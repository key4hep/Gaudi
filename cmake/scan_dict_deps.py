#!/usr/bin/env python3
#####################################################################################
# (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################

import io
import re
from os.path import exists, isabs, isdir, join

INCLUDE_RE = re.compile(r'^\s*#\s*include\s*["<]([^">]*)[">]')


def find_file(filename, searchpath):
    """
    Return the absolute path to filename in the searchpath.

    If filename is already an absolute path, return it as is, if it exists.

    If filename cannot be found, return None.
    """
    if isabs(filename):
        return filename if exists(filename) else None

    for f in (join(d, filename) for d in searchpath):
        if exists(f):
            return f

    return None


def find_deps(filename, searchpath, deps=None):
    """
    Return a set with the absolute paths to the files included (directly and
    indirectly) by filename.
    """
    if deps is None:
        deps = set()

    filename = find_file(filename, searchpath)
    if not filename:
        # ignore missing files (useful for generated .h files)
        return deps

    # Look for all "#include" lines in the file, then consider each of the
    # included files, ignoring those already included in the recursion
    for included in [
        f
        for f in [
            find_file(m.group(1), searchpath)
            for m in [INCLUDE_RE.match(l) for l in io.open(filename, encoding="utf-8")]
            if m
        ]
        if f and f not in deps and not isdir(f)
    ]:
        deps.add(included)
        find_deps(included, searchpath, deps)

    return deps


def main():
    from argparse import ArgumentParser

    parser = ArgumentParser()
    parser.add_argument(
        "-I",
        action="append",
        dest="include_dirs",
        help="directories where to look for header files",
    )
    parser.add_argument(
        "output_file",
        help="name of the files to write (will be updated only if there's a change)",
    )
    parser.add_argument(
        "target", help="build target to be rebuilt if the dependencies change"
    )
    parser.add_argument("headers", help="header files to process", nargs="+")

    args = parser.parse_args()

    if exists(args.output_file):
        with open(args.output_file) as f:
            old_deps = f.read()
    else:
        old_deps = None

    # scan for dependencies
    deps = set()
    for filename in args.headers:
        find_deps(filename, args.include_dirs, deps)
    deps = sorted(deps)

    # prepare content of output file
    new_deps = "{target}: {deps}\n".format(target=args.target, deps=" ".join(deps))

    if new_deps != old_deps:  # write it only if it has changed
        with open(args.output_file, "w") as f:
            f.write(new_deps)


if __name__ == "__main__":
    main()

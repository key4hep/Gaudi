#!/usr/bin/env python
import re

from itertools import imap, ifilter
from os.path import join, exists, isabs


def find_file(filename, searchpath):
    '''
    Return the absolute path to filename in the searchpath.

    If filename is already an absolute path, return it as is, if it exists.

    If filename cannot be found, return None.
    '''
    if isabs(filename):
        return filename if exists(filename) else None
    try:
        return ifilter(exists, imap(lambda x: join(x, filename),
                                    searchpath)).next()
    except StopIteration:
        return None


def find_deps(filename, searchpath, deps=None):
    '''
    Return a set with the absolute paths to the files included (directly and
    indirectly) by filename.
    '''
    if deps is None:
        deps = set()

    filename = find_file(filename, searchpath)
    if not filename:
        # ignore missing files (useful for generated .h files)
        return deps

    # Look for all "#include" lines in the file, then consider each of the
    # included files, ignoring those already included in the recursion
    for included in ifilter(lambda f: f and f not in deps,
                            imap(lambda m: m and find_file(m.group(1), searchpath),
                                 imap(re.compile(r'^\s*#\s*include\s*["<]([^">]*)[">]').match,
                                      open(filename)))):
        deps.add(included)
        find_deps(included, searchpath, deps)

    return deps


def main():
    from optparse import OptionParser

    parser = OptionParser(
        usage='%prog [options] output_file variable_name headers...')
    parser.add_option('-I', action='append', dest='include_dirs')

    opts, args = parser.parse_args()
    if len(args) < 2:
        parser.error('you must specify output file and variable name')

    output, variable = args[:2]
    headers = args[2:]

    old_deps = open(output).read() if exists(output) else None

    # scan for dependencies
    deps = set()
    for filename in headers:
        find_deps(filename, opts.include_dirs, deps)
    deps = sorted(deps)

    # prepare content of output file
    new_deps = 'set({deps_var}\n    {deps})\n' \
        .format(deps='\n    '.join(deps), deps_var=variable)

    if new_deps != old_deps:  # write it only if it has changed
        open(output, 'w').write(new_deps)
        if old_deps:
            print 'info: dependencies changed: next build will trigger a reconfigure'


if __name__ == '__main__':
    main()

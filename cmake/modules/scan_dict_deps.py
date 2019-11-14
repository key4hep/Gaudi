#!/usr/bin/env python
from __future__ import print_function
import re

import six
import io
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
        return six.next(
            six.moves.filter(
                exists, six.moves.map(lambda x: join(x, filename),
                                      searchpath)))
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
    for included in six.moves.filter(
            lambda f: f and f not in deps,
            six.moves.map(
                lambda m: m and find_file(m.group(1), searchpath),
                six.moves.map(
                    re.compile(r'^\s*#\s*include\s*["<]([^">]*)[">]').match,
                    io.open(filename, encoding="utf-8")))):
        deps.add(included)
        find_deps(included, searchpath, deps)

    return deps


def main():
    from optparse import OptionParser

    parser = OptionParser(
        usage='%prog [options] output_file variable_name headers...')
    parser.add_option('-I', action='append', dest='include_dirs')
    parser.add_option(
        '-M',
        '--for-make',
        action='store_true',
        help='generate Makefile like dependencies (as with gcc '
        '-MD) in which case "variable_name" is the name of the '
        'target')

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
    if opts.for_make:
        new_deps = '{target}: {deps}\n'.format(
            target=variable, deps=' '.join(deps))
    else:
        new_deps = 'set({deps_var}\n    {deps})\n' \
            .format(deps='\n    '.join(deps), deps_var=variable)

    if new_deps != old_deps:  # write it only if it has changed
        open(output, 'w').write(new_deps)
        if old_deps and not opts.for_make:
            print(
                'info: dependencies changed: next build will trigger a reconfigure'
            )


if __name__ == '__main__':
    main()

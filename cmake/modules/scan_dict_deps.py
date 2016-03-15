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

    parser = OptionParser()
    parser.add_option('-I', action='append', dest='include_dirs')
    parser.add_option('-t', '--target')
    parser.add_option('-o', '--output')

    opts, args = parser.parse_args()

    with open(opts.output, 'w') as f:
        for filename in args:
            #f.write('''message(STATUS "Dependencies for %s")\n''' % filename)
            deps = find_deps(filename, opts.include_dirs)
            #for dep in sorted(deps):
            #    f.write('''message(STATUS "  - %s")\n''' % dep)
            f.write('''
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
   {deps})
set({target}FileDeps DEPENDS
   {deps})
'''
                    .format(deps='\n   '.join(deps), target=opts.target))

if __name__ == '__main__':
    main()

#!/usr/bin/env python
'''
Simple script to extract the dependencies of QMTest tests (.qmt files) and
report them as declaration of CTest dependencies.
'''
__author__ = 'Marco Clemencic <marco.clemencic@cern.ch>'

import os

def qmt_filename_to_name(path):
    '''
    convert the relative path to a .qmt file to the canonical QMTest test name.

    For example:

    >>> qmt_filename_to_name('some_suite.qms/sub.qms/mytest.qmt')
    'some_suite.sub.mytest'
    '''
    import re
    return '.'.join(re.sub(r'\.qm[st]$', '', p)
                    for p in path.split(os.path.sep))

def analize_deps(pkg, rootdir):
    '''
    Collect dependencies from the QMTest tests in a directory and report them
    to stdout as CMake commands.

    @param pkg: name of the package (used to fix the name of the tests to match
                the CMake ones
    @param rootdir: directory containing the QMTest tests (usually tests/qmtest)
    '''
    from os.path import join, relpath
    import xml.etree.ElementTree as ET

    pkg_cmake_prefix = pkg + '.'
    pkg_qmt_prefix = pkg.lower() + '.'

    prereq_xpath = 'argument[@name="prerequisites"]/set/tuple/text'
    for dirpath, dirnames, filenames in os.walk(rootdir):
        for filename in filenames:
            if filename.endswith('.qmt'):
                path = join(dirpath, filename)
                name = qmt_filename_to_name(relpath(path, rootdir))
                name = name.replace(pkg_qmt_prefix, pkg_cmake_prefix)

                tree = ET.parse(path)
                prereqs = [el.text.replace(pkg_qmt_prefix, pkg_cmake_prefix)
                           for el in tree.findall(prereq_xpath)]
                if prereqs:
                    print ('set_property(TEST {0} APPEND PROPERTY DEPENDS {1})'
                           .format(name, ' '.join(prereqs)))


if __name__ == '__main__':
    import sys
    analize_deps(*sys.argv[1:])

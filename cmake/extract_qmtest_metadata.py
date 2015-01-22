#!/usr/bin/env python
'''
Simple script to extract metadata (dependencies, labels) from QMTest tests (.qmt
files) and suites (.qms files), and report them as declaration of CTest test
properties.
'''
__author__ = 'Marco Clemencic <marco.clemencic@cern.ch>'

import os
import xml.etree.ElementTree as ET
import collections

def qmt_filename_to_name(path):
    '''
    convert the relative path to a .qmt/.qms file to the canonical QMTest test
    name.

    For example:

    >>> qmt_filename_to_name('some_suite.qms/sub.qms/mytest.qmt')
    'some_suite.sub.mytest'
    '''
    import re
    return '.'.join(re.sub(r'\.qm[st]$', '', p)
                    for p in path.split(os.path.sep))

def find_files(rootdir, ext):
    '''
    Find recursively all the files in a directory with a given extension.
    '''
    for dirpath, _dirnames, filenames in os.walk(rootdir):
        for filename in filenames:
            if os.path.splitext(filename)[1] == ext:
                yield os.path.join(dirpath, filename)


def analyze_deps(pkg, rootdir):
    '''
    Collect dependencies from the QMTest tests in a directory and report them
    to stdout as CMake commands.

    @param pkg: name of the package (used to fix the name of the tests to match
                the CMake ones
    @param rootdir: directory containing the QMTest tests (usually tests/qmtest)
    '''
    pkg_cmake_prefix = pkg + '.'
    pkg_qmt_prefix = pkg.lower() + '.'

    prereq_xpath = 'argument[@name="prerequisites"]/set/tuple/text'
    for path in find_files(rootdir, '.qmt'):
        name = qmt_filename_to_name(os.path.relpath(path, rootdir))
        name = name.replace(pkg_qmt_prefix, pkg_cmake_prefix)

        tree = ET.parse(path)
        prereqs = [el.text.replace(pkg_qmt_prefix, pkg_cmake_prefix)
                   for el in tree.findall(prereq_xpath)]
        if prereqs:
            print ('set_property(TEST {0} APPEND PROPERTY DEPENDS {1})'
                   .format(name, ' '.join(prereqs)))

def analyze_suites(pkg, rootdir):
    '''
    Find all the suites (.qms files) defined in a directory and use it as a
    label for the tests in it.
    '''
    pkg_cmake_prefix = pkg + '.'
    pkg_qmt_prefix = pkg.lower() + '.'

    labels = collections.defaultdict(list)

    tests_xpath = 'argument[@name="test_ids"]/set/text'
    suites_xpath = 'argument[@name="suite_ids"]/set/text'
    for path in find_files(rootdir, '.qms'):
        name = qmt_filename_to_name(os.path.relpath(path, rootdir))
        name = name.replace(pkg_qmt_prefix, pkg_cmake_prefix)

        tree = ET.parse(path)

        labels[name].extend(el.text.replace(pkg_qmt_prefix, pkg_cmake_prefix)
                            for el in tree.findall(tests_xpath))

        if tree.findall(suites_xpath):
            sys.stderr.write(('WARNING: %s: suites of suites are '
                              'not supported yet\n') % path)
            sys.stderr.flush()

    # transpose the dictionary of lists
    test_labels = collections.defaultdict(set)
    for label, tests in labels.iteritems():
        for test in tests:
            test_labels[test].add(label)

    for test, labels in test_labels.iteritems():
        print ('set_property(TEST {0} APPEND PROPERTY LABELS {1})'
               .format(test, ' '.join(labels)))


if __name__ == '__main__':
    import sys
    analyze_deps(*sys.argv[1:])
    analyze_suites(*sys.argv[1:])

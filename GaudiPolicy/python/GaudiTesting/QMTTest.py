# -*- coding: utf-8 -*-
from BaseTest import *
import logging
import os
import sys

class QMTTest(BaseTest):

    def __init__(self, path=None):
        BaseTest.__init__(self)
        self.validator = ''
        if path:
            self.XMLParser(path)

    def XMLParser(self, path) :
        '''
        Parse a QMTest XML test description (.qmt file) to initialize the test
        instance.
        '''
        log = logging.getLogger('QMTest.XMLParser')
        import xml.etree.ElementTree as ET
        log.debug('parsing %s', path)

        self.name = path

        tree = ET.parse(path)
        for child in tree.getroot():
            name = child.attrib['name']
            if hasattr(self, name):
                log.debug('setting %s', name)
                value = child[0]
                if name in ('args', 'unsupported_platforms'):
                    setattr(self, name, [el.text
                                         for el in value.findall('text')])
                elif name == 'environment':
                    setattr(self, name, dict(el.text.split('=', 1)
                                             for el in value.findall('text')))
                else:
                    data = value.text
                    if value.tag == 'integer':
                        data = int(data)
                    setattr(self, name, data)


    def ValidateOutput(self, stdout, stderr, result):
        if self.validator != '' :
            class CallWrapper(object):
                """
                    Small wrapper class to dynamically bind some default arguments
                    to a callable.
                    """
                def __init__(self, callable, extra_args = {}):
                    self.callable = callable
                    self.extra_args = extra_args
                    # get the list of names of positional arguments
                    from inspect import getargspec
                    self.args_order = getargspec(callable)[0]
                    # Remove "self" from the list of positional arguments
                    # since it is added automatically
                    if self.args_order[0] == "self":
                        del self.args_order[0]
                def __call__(self, *args, **kwargs):
                    # Check which positional arguments are used
                    positional = self.args_order[:len(args)]

                    kwargs = dict(kwargs) # copy the arguments dictionary
                    for a in self.extra_args:
                        # use "extra_args" for the arguments not specified as
                        # positional or keyword
                        if a not in positional and a not in kwargs:
                            kwargs[a] = self.extra_args[a]
                    return apply(self.callable, args, kwargs)

            # local names to be exposed in the script
            exported_symbols = {"self":self,
                                "stdout":stdout,
                                "stderr":stderr,
                                "result":result,
                                "causes":self.causes,
                                "findReferenceBlock":
                                    CallWrapper(self.findReferenceBlock, {"stdout":stdout,
                                                                     "result":result,
                                                                     "causes":self.causes}),
                                "validateWithReference":
                                    CallWrapper(self.validateWithReference, {"stdout":stdout,
                                                                        "stderr":stderr,
                                                                        "result":result,
                                                                        "causes":self.causes}),
                                "countErrorLines":
                                    CallWrapper(self.countErrorLines, {"stdout":stdout,
                                                                  "result":result,
                                                                  "causes":self.causes}),
                                "checkTTreesSummaries":
                                    CallWrapper(self.CheckTTreesSummaries, {"stdout":stdout,
                                                                       "result":result,
                                                                       "causes":self.causes}),
                                "checkHistosSummaries":
                                    CallWrapper(self.CheckHistosSummaries, {"stdout":stdout,
                                                                       "result":result,
                                                                       "causes":self.causes})
                                }
            exec self.validator in globals(), exported_symbols
        else:
            if self.stderr=='':
                self.validateWithReference(stdout, stderr, result, self.causes)
            elif stderr!=self.stderr:
                self.causes.append("DIFFERENT STDERR THAN EXPECTED")

        return result,self.causes


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
    analize_deps(*sys.argv[1:])

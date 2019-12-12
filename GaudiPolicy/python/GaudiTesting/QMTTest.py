# -*- coding: utf-8 -*-
#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiTesting.BaseTest import *
import logging
import os
import sys


class QMTTest(BaseTest):
    def __init__(self, path=None):
        BaseTest.__init__(self)
        self.validator = ''
        if path:
            self.XMLParser(path)

    def XMLParser(self, path):
        '''
        Parse a QMTest XML test description (.qmt file) to initialize the test
        instance.
        '''
        log = logging.getLogger('QMTest.XMLParser')
        import xml.etree.ElementTree as ET
        log.debug('parsing %s', path)

        self.name = '.'.join(
            os.path.relpath(path, self.basedir).replace('.qmt', '').replace(
                '.qms', '').split(os.sep))

        tree = ET.parse(path)
        for child in tree.getroot():
            name = child.attrib['name']
            if hasattr(self, name):
                log.debug('setting %s', name)
                value = child[0]
                if name in ('args', 'unsupported_platforms'):
                    setattr(self, name,
                            [el.text for el in value.findall('text')])
                elif name == 'environment':
                    setattr(
                        self, name,
                        dict(
                            el.text.split('=', 1)
                            for el in value.findall('text')))
                else:
                    data = value.text
                    if data is not None:
                        if value.tag == 'integer':
                            data = int(data)
                        setattr(self, name, data)

    def ValidateOutput(self, stdout, stderr, result):
        if self.validator:

            class CallWrapper(object):
                """
                    Small wrapper class to dynamically bind some default arguments
                    to a callable.
                    """

                def __init__(self, callable, extra_args={}):
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

                    kwargs = dict(kwargs)  # copy the arguments dictionary
                    for a in self.extra_args:
                        # use "extra_args" for the arguments not specified as
                        # positional or keyword
                        if a not in positional and a not in kwargs:
                            kwargs[a] = self.extra_args[a]
                    return self.callable(*args, **kwargs)

            # local names to be exposed in the script
            stdout_ref = self._expandReferenceFileName(self.reference)
            stderr_ref = self._expandReferenceFileName(self.error_reference)
            exported_symbols = {
                "self":
                self,
                "stdout":
                stdout,
                "stderr":
                stderr,
                "result":
                result,
                "causes":
                self.causes,
                "reference":
                stdout_ref,
                "error_reference":
                stderr_ref,
                "findReferenceBlock":
                CallWrapper(self.findReferenceBlock, {
                    "stdout": stdout,
                    "result": result,
                    "causes": self.causes
                }),
                "validateWithReference":
                CallWrapper(
                    self.validateWithReference, {
                        "stdout": stdout,
                        "stderr": stderr,
                        "result": result,
                        "causes": self.causes
                    }),
                "countErrorLines":
                CallWrapper(self.countErrorLines, {
                    "stdout": stdout,
                    "result": result,
                    "causes": self.causes
                }),
                "checkTTreesSummaries":
                CallWrapper(self.CheckTTreesSummaries, {
                    "stdout": stdout,
                    "result": result,
                    "causes": self.causes
                }),
                "checkHistosSummaries":
                CallWrapper(self.CheckHistosSummaries, {
                    "stdout": stdout,
                    "result": result,
                    "causes": self.causes
                })
            }
            # print self.validator
            exec (self.validator, globals(), exported_symbols)
            return result, self.causes
        else:
            return super(QMTTest, self).ValidateOutput(stdout, stderr, result)

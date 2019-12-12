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

import os
from os.path import join, dirname

from cmake_test_utils import CMakeTestScripts


class Tests(CMakeTestScripts):
    base_dir = dirname(__file__)
    scripts_dir = join(base_dir, 'cmake_scripts')

    def setup(self):
        os.environ['CMAKE_PREFIX_PATH'] = join(self.base_dir, 'data',
                                               'toolchain')
        os.environ['BINARY_TAG'] = 'x86_64-slc0-gcc99-opt'

    tests = [
        'toolchain_extensions_min', 'toolchain_extensions',
        'toolchain_extensions_multi'
    ]

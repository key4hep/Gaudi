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
        # There is no 'projects_cmt' directory, but it helps testing the priorities
        # of CMAKE_PREFIX_PATH and CMTPROJECTPATH
        os.environ['CMTPROJECTPATH'] = join(self.base_dir, 'data',
                                            'projects_cmt')
        os.environ['CMAKE_PREFIX_PATH'] = join(self.base_dir, 'data',
                                               'projects')
        #os.environ['CMTPROJECTPATH'] = ''
        #os.environ['CMAKE_PREFIX_PATH'] = join(self.base_dir, 'data', 'projects')
        os.environ['BINARY_TAG'] = 'x86_64-slc0-gcc99-opt'

    tests = [
        'no_use', 'simple_use', 'chain', 'diamond', 'with_tools',
        'with_chained_tools', 'version_selection', 'atlas_convention',
        'special_conventions', 'guess_toolchain'
    ]

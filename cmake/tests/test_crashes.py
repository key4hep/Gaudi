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
from __future__ import print_function
from cmake_coverage import cmake_script
from os.path import join, dirname

base_dir = dirname(__file__)
scripts_dir = join(base_dir, 'cmake_scripts')


def test_loops():
    script_name = join(scripts_dir, 'test_loops.cmake')
    out, err, returncode = cmake_script(script_name, cwd=base_dir)
    print("---------- stdout ----------")
    print(out)
    print("---------- stderr ----------")
    print(err)
    assert returncode != 0
    assert 'Infinite recursion detected at project Loop' in err


def test_missing_base():
    script_name = join(scripts_dir, 'test_missing_base.cmake')
    out, err, returncode = cmake_script(script_name, cwd=base_dir)
    print("---------- stdout ----------")
    print(out)
    print("---------- stderr ----------")
    print(err)
    assert returncode != 0
    assert 'Cannot find project IDoNotExist v0r0' in err

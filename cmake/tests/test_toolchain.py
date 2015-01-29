# -*- coding: utf-8 -*-

import os
from os.path import join, dirname

from cmake_coverage import cmake_script

base_dir = dirname(__file__)

def setup():
    os.environ['CMAKE_PREFIX_PATH'] = join(base_dir, 'data', 'toolchain')
    os.environ['CMTCONFIG'] = 'x86_64-slc6-gcc48-opt'

def cmake_script_runner(name):
    script_name = join(base_dir, 'cmake_scripts', 'test_%s.cmake' % name)
    out, err, returncode = cmake_script(script_name, cwd=base_dir)
    print "---------- stdout ----------"
    print out
    print "---------- stderr ----------"
    print err
    assert returncode == 0

def test_script():
    for test in ['toolchain_extensions_min',
                 'toolchain_extensions',
                 'toolchain_extensions_multi']:
        yield cmake_script_runner, test

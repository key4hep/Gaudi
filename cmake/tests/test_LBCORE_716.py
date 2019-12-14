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
'''
Test to check the correct override order of projects.
'''
import os
import re
from subprocess import Popen, PIPE

base_dir = os.path.realpath(
    os.path.join(os.path.dirname(__file__), 'data', 'LBCORE-716'))

xenv_cmd = 'xenv'

build_log = None
build_returncode = None
build_err = None


def clean():
    Popen(['make', 'clean'], cwd=base_dir, stdout=PIPE,
          stderr=PIPE).communicate()


def build():
    global build_log, build_returncode, build_err
    for v in ('BINARY_TAG', 'CMTCONFIG'):
        if v in os.environ:
            del os.environ[v]
    import sys
    sys.path.append(os.path.dirname(os.path.dirname(__file__)))
    from get_host_binary_tag import os_id, compiler_id
    os.environ['BINARY_TAG'] = '-'.join(
        ['x86_64', os_id(), compiler_id(), 'opt'])
    build_proc = Popen(['make', 'VERBOSE=1'],
                       cwd=base_dir,
                       stdout=PIPE,
                       stderr=PIPE)
    build_log, build_err = build_proc.communicate()
    build_log, build_err = build_log.decode('utf-8'), build_err.decode('utf-8')
    build_returncode = build_proc.returncode


def setup():
    clean()
    build()


def test_build():
    assert build_returncode == 0

    out = build_log.splitlines()

    compile_cmd = [l for l in out if re.match(r'.*PkgD.*code\.cpp$', l)]
    assert len(compile_cmd) == 1
    compile_cmd = compile_cmd[0]

    include_order = [
        os.path.relpath(l[2:], base_dir)[0] for l in compile_cmd.split()
        if l.startswith('-I') and 'InstallArea' in l
    ]
    assert include_order == ['B', 'C', 'A'], include_order


def test_env():
    assert build_returncode == 0

    envfile = os.path.join(base_dir, 'D', 'InstallArea', 'D.xenv')
    assert os.path.exists(envfile)

    getenv = Popen(
        [xenv_cmd, '--xml', envfile, 'printenv', 'ROOT_INCLUDE_PATH'],
        stdout=PIPE,
        stderr=PIPE)
    out, _err = getenv.communicate()
    out = out.decode('utf-8')
    assert getenv.returncode == 0, getenv.returncode

    root_inc_path = [
        os.path.relpath(l, base_dir)[0] for l in out.strip().split(os.pathsep)
        if 'InstallArea' in l
    ]
    # if we run the test after "make install" is called, we get one extra '.'
    # in the list (because of the InstallArea of Gaudi itself)
    root_inc_path = [l for l in root_inc_path if l != '.']
    assert root_inc_path == ['D', 'B', 'C', 'A'], root_inc_path


def teardown():
    clean()

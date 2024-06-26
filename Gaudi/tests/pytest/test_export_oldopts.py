#!/usr/bin/env python3
#####################################################################################
# (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################

import os
from ast import literal_eval
from pathlib import Path
from shutil import which
from subprocess import run

import pytest


@pytest.mark.parametrize("optfiles", [["main.py"], ["export_oldopts/special_cases.py"]])
def test(optfiles, tmp_path):
    env = dict(os.environ)
    env["JOBOPTSEARCHPATH"] = str(Path(__file__).parent.parent / "pyjobopts")

    print("======== First pass (python only) ========")
    # parse the option file and cache the configuration (python only)
    dump_name = str(tmp_path / "options.1.py")
    cmd = [
        "python3",
        which("gaudirun.py"),
        "--dry-run",
        "--verbose",
        "--output",
        dump_name,
    ] + optfiles
    print("= cmd:", " ".join(cmd))
    run(cmd, env=env, check=True)
    expected = literal_eval(open(dump_name).read())

    print("========== Second pass (export) ==========")
    # parse the option file, export old options, parse again
    export_name = str(tmp_path / "options.opts")
    cmd = [
        "python3",
        which("gaudirun.py"),
        "--dry-run",
        "--verbose",
        "--old-opts",
        "--output",
        export_name,
    ] + optfiles
    print("= cmd:", " ".join(cmd))
    run(cmd, env=env, check=True)

    print("========== Second pass (import) ==========")
    dump_name = str(tmp_path / "options.2.py")
    cmd = [
        "python3",
        which("gaudirun.py"),
        "-n",
        "-v",
        "--output",
        dump_name,
        export_name,
    ]
    print("= cmd:", " ".join(cmd))
    run(cmd, env=env, check=True)
    result = literal_eval(open(dump_name).read())

    assert (
        result == expected
    ), "Configuration from old options differs from the python one"

#####################################################################################
# (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import os
from pathlib import Path

import pytest
from GaudiTesting import utils


@pytest.mark.parametrize(
    "reference,platform,files,expected",
    [
        # unstructured
        (
            "refs/my.ref",
            "x86_64-el9-gcc13-opt",
            ["my.ref", "my.ref.dbg"],
            "refs/my.ref",
        ),
        (
            "refs/my.ref",
            "x86_64-el9-gcc13-dbg",
            ["my.ref", "my.ref.dbg"],
            "refs/my.ref.dbg",
        ),
        (
            "refs/my.ref",
            "x86_64-el9-gcc13-dbg",
            ["my.ref", "my.ref.dbg", "my.ref.dbg-centos7"],
            "refs/my.ref.dbg",
        ),
        (
            "refs/my.ref",
            "x86_64-el9-gcc13-dbg",
            ["my.ref", "my.ref.dbg-centos7"],
            "refs/my.ref",
        ),
        (
            "refs/my.ref",
            "x86_64-centos7-gcc13-dbg",
            ["my.ref", "my.ref.dbg", "my.ref.dbg-centos7"],
            "refs/my.ref.dbg-centos7",
        ),
        # structured
        (
            "refs/my_ref.yaml",
            "x86_64-el9-gcc13-opt",
            ["my_ref.yaml", "my_ref.dbg.yaml"],
            "refs/my_ref.yaml",
        ),
        (
            "refs/my_ref.yaml",
            "x86_64-el9-gcc13-dbg",
            ["my_ref.yaml", "my_ref.dbg.yaml"],
            "refs/my_ref.dbg.yaml",
        ),
        (
            "refs/my_ref.yaml",
            "x86_64-el9-gcc13-dbg",
            ["my_ref.yaml", "my_ref.dbg.yaml", "my_ref.dbg-centos7.yaml"],
            "refs/my_ref.dbg.yaml",
        ),
        (
            "refs/my_ref.yaml",
            "x86_64-el9-gcc13-dbg",
            ["my_ref.yaml", "my_ref.dbg-centos7.yaml"],
            "refs/my_ref.yaml",
        ),
        (
            "refs/my_ref.yaml",
            "x86_64-centos7-gcc13-dbg",
            ["my_ref.yaml", "my_ref.dbg.yaml", "my_ref.dbg-centos7.yaml"],
            "refs/my_ref.dbg-centos7.yaml",
        ),
        (
            "refs/my_ref.yml",
            "x86_64-el9-gcc13-dbg",
            ["my_ref.yml", "my_ref.dbg.yml"],
            "refs/my_ref.dbg.yml",
        ),
        # invalid override
        (
            "refs/my_ref.yaml",
            "x86_64-el9-gcc13-dbg",
            ["my_ref.yaml", "my_ref.yaml.dbg"],
            "refs/my_ref.yaml",
        ),
        # missing
        ("refs/my_ref.yaml", "x86_64-el9-gcc13-opt", [], "refs/my_ref.yaml"),
    ],
)
def test_expand_reference_file(reference, platform, files, expected, monkeypatch):
    monkeypatch.setattr(utils, "get_platform", lambda: platform)
    monkeypatch.setattr(os, "listdir", lambda _: files)
    monkeypatch.setattr(os.path, "isdir", lambda _: True)

    assert utils.expand_reference_file_name(reference) == expected


class Dummy:
    pass


def test_file_for_class():
    assert utils.file_path_for_class(Dummy) == Path(__file__)

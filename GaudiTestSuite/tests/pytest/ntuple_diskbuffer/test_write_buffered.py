#####################################################################################
# (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import math
import re

import pytest
from GaudiTesting import GaudiExeTest


def tree_shape(tree):
    """Everything about a tree except its values."""
    leaves = []
    for br in tree.GetListOfBranches():
        for leaf in br.GetListOfLeaves():
            count = leaf.GetLeafCount()
            leaves.append(
                (
                    br.GetName(),
                    br.GetTitle(),
                    leaf.GetName(),
                    leaf.GetTypeName(),
                    leaf.GetLenStatic(),
                    count.GetName() if count else None,
                    leaf.IsRange(),
                    leaf.GetMaximum(),
                    leaf.GetMinimum(),
                )
            )
    return (tree.GetName(), tree.GetTitle(), tree.GetEntries(), leaves)


def tree_values(tree):
    """All values of all leaves, entry by entry, up to each leaf's current length."""
    leaves = [leaf for br in tree.GetListOfBranches() for leaf in br.GetListOfLeaves()]
    rows = []
    for i in range(tree.GetEntries()):
        tree.GetEntry(i)
        rows.append(
            [[leaf.GetValue(k) for k in range(leaf.GetLen())] for leaf in leaves]
        )
    return rows


def same_values(a, b):
    if isinstance(a, list):
        return len(a) == len(b) and all(same_values(x, y) for x, y in zip(a, b))
    return a == b or (math.isnan(a) and math.isnan(b))


def walk(directory, path=""):
    """Yield (path, class name, object) for every key, recursing into directories."""
    for key in directory.GetListOfKeys():
        obj = key.ReadObj()
        name = path + "/" + key.GetName()
        yield name, key.GetClassName(), obj
        if obj.InheritsFrom("TDirectory"):
            yield from walk(obj, name)


@pytest.mark.ctest_fixture_required("ntuple_diskbuffer_columnar")
@pytest.mark.shared_cwd("ntuple_diskbuffer")
class Test(GaudiExeTest):
    """Same job with NTupleSvc.DiskBuffer: the output must match the columnar run."""

    command = [
        "gaudirun.py",
        "../../../options/NTupleDiskBuffer.py",
        "--option",
        "NTupleSvc().Output = [\"MyTuples DATAFILE='ntuple_buffered.root' OPT='NEW' TYP='ROOT'\"]",
        "--option",
        "NTupleSvc().DiskBuffer = True",
    ]

    def test_messages(self, stdout):
        out = stdout.decode()
        assert "ERROR" not in out
        # 6 wide + 1 deep + the column-wise example; the empty one takes the direct path
        assert len(re.findall(r"RCWNTupleCnv +INFO Buffering TTree", out)) == 8
        assert len(re.findall(r"RCWNTupleCnv +INFO Built TTree", out)) == 8
        assert "RCWNTupleCnv          INFO Booked TTree with ID: t0" in out
        assert "empty ntuple" in out

    def test_no_leftover(self, cwd):
        assert not list(cwd.glob("*.ntbuf"))

    def test_equivalent(self, cwd):
        import ROOT

        columnar = ROOT.TFile.Open(str(cwd / "ntuple_columnar.root"))
        buffered = ROOT.TFile.Open(str(cwd / "ntuple_buffered.root"))
        try:
            ref = {path: (cls, obj) for path, cls, obj in walk(columnar)}
            new = {path: (cls, obj) for path, cls, obj in walk(buffered)}
            assert sorted(new) == sorted(ref)
            trees = 0
            for path, (cls, obj) in ref.items():
                assert new[path][0] == cls, path
                if cls != "TTree":
                    continue
                trees += 1
                assert tree_shape(new[path][1]) == tree_shape(obj), path
                assert same_values(tree_values(new[path][1]), tree_values(obj)), path
            assert trees == 10
        finally:
            columnar.Close()
            buffered.Close()

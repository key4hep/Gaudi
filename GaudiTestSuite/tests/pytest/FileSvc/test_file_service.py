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

import pytest
import ROOT
from GaudiConfig2 import Configurables as C
from GaudiTests import run_gaudi

OUTPUT_FILE_NAME = "file_svc_shared.root"


@pytest.fixture(scope="module")
def setup_file_tree(tmp_path_factory):
    tmp_dir = tmp_path_factory.mktemp("FileSvc")
    os.chdir(tmp_dir)
    if os.path.exists(OUTPUT_FILE_NAME):
        os.remove(OUTPUT_FILE_NAME)

    run_gaudi(f"{__file__}:config", check=True)

    file = ROOT.TFile.Open(OUTPUT_FILE_NAME)
    tree = file.Get("FileSvcNTuple")
    hist = file.Get("FileSvcRandomHist")

    yield file, tree, hist

    file.Close()


def config():
    E = C.Gaudi.TestSuite.FileSvc

    algs = [
        E.NTupleWriterAlg("NTupleWriterAlg"),
        E.HistogramWriterAlg("HistogramWriterAlg"),
    ]

    fileSvc = C.FileSvc(
        Config={
            "Histogram": f"{OUTPUT_FILE_NAME}?mode=recreate&flag=test",
            "Ntuple": f"{OUTPUT_FILE_NAME}?mode=recreate&flag=test",
        }
    )

    loopmgr = C.HiveSlimEventLoopMgr(SchedulerName="AvalancheSchedulerSvc")
    whiteboard = C.HiveWhiteBoard("EventDataSvc", EventSlots=5)
    svcs = [
        fileSvc,
        whiteboard,
    ]
    return (
        [
            C.ApplicationMgr(
                TopAlg=algs,
                EvtMax=10,
                EvtSel="NONE",
                ExtSvc=svcs,
                EventLoop=loopmgr.name,
            ),
            loopmgr,
        ]
        + algs
        + svcs
        + [fileSvc]
    )


def test_ntuple_exists(setup_file_tree):
    _, tree, _ = setup_file_tree
    assert tree is not None, "FileSvcNTuple should exist in the output file"
    assert tree.GetBranch("value"), "Branch `value` should exist in FileSvcNTuple."
    assert tree.GetEntries() > 0, "FileSvcNTuple should have entries"


def test_histogram_exists(setup_file_tree):
    _, _, hist = setup_file_tree
    assert hist is not None, "FileSvcRandomHist should exist in the output file"
    assert hist.GetEntries() > 0, "FileSvcRandomHist should have entries"


def test_file_not_zombie(setup_file_tree):
    file, _, _ = setup_file_tree
    assert not file.IsZombie(), "Output file should not be a zombie"

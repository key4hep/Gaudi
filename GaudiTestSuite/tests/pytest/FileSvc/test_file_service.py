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
import pytest
import ROOT
from GaudiTesting import GaudiExeTest

OUTPUT_FILE_NAME = "file_svc_shared.root"


class Test(GaudiExeTest):
    command = ["gaudirun.py", f"{__file__}:config"]

    @pytest.fixture(scope="class")
    def setup_file_tree(self, cwd):
        file = ROOT.TFile.Open(str(cwd / OUTPUT_FILE_NAME))
        tree = file.Get("FileSvcNTuple")
        hist = file.Get("FileSvcRandomHist")
        yield file, tree, hist
        file.Close()

    def test_ntuple_exists(self, setup_file_tree):
        _, tree, _ = setup_file_tree
        assert tree is not None, "FileSvcNTuple should exist in the output file"
        assert tree.GetBranch("value"), "Branch `value` should exist in FileSvcNTuple."
        assert tree.GetEntries() > 0, "FileSvcNTuple should have entries"

    def test_histogram_exists(self, setup_file_tree):
        _, _, hist = setup_file_tree
        assert hist is not None, "FileSvcRandomHist should exist in the output file"
        assert hist.GetEntries() > 0, "FileSvcRandomHist should have entries"

    def test_file_not_zombie(self, setup_file_tree):
        file, _, _ = setup_file_tree
        assert not file.IsZombie(), "Output file should not be a zombie"


def config():
    from GaudiConfig2 import Configurables as C

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

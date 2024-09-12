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
import inspect
import os

import pytest
from GaudiTesting import GaudiExeTest

FILENAME = "custom_axis.root"
FILENAMEJSON = "custom_axis.json"


def config():
    import GaudiConfig2.Configurables as C
    import GaudiConfig2.Configurables.Gaudi.Tests.Histograms.CustomAxis as T

    algs = []
    tools = []
    svcs = []

    Alg = T.EnumAxisAlg
    algs.append(Alg("Alg"))

    svcs.append(C.Gaudi.Histograming.Sink.Root(FileName=FILENAME))
    svcs.append(C.Gaudi.Monitoring.JSONSink(FileName=FILENAMEJSON))
    svcs.append(C.Gaudi.Monitoring.MessageSvcSink())

    yield from algs
    yield from tools
    yield from svcs

    yield C.ApplicationMgr(
        EvtMax=5,
        EvtSel="NONE",
        TopAlg=algs,
        ExtSvc=svcs,
    )


class TestCustomAxis(GaudiExeTest):
    currentFile = inspect.getfile(inspect.currentframe())
    command = ["gaudirun.py", f"{currentFile}:config"]

    @pytest.fixture(autouse=True)
    def changeDir(self, monkeypatch):
        monkeypatch.chdir(self.cwd)
        yield

    def test_axis_labels(self):
        assert os.path.exists(FILENAME)

        import ROOT

        f = ROOT.TFile.Open(FILENAME)
        h = f.Get("Alg/Categories")
        if not h:
            k = f.GetKey("Alg/Categories")
            if k:
                h = k.ReadObj()

        assert h
        assert h.GetNbinsX() == 4

        axis = h.GetXaxis()
        labels = list(axis.GetLabels())
        expected = ["Simple", "Complex", "Bad", "Wrong"]
        assert labels == expected

    def validate_json_with_reference(self):
        self.validate_json_with_reference(
            FILENAMEJSON, "../refs/histograms/custom_axis.json"
        )

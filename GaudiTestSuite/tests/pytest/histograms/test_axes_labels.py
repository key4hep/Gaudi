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

from GaudiTesting import GaudiExeTest

FILENAME = "axes_labels.root"
FILENAMEJSON = "axes_labels.json"


def config():
    import GaudiConfig2.Configurables as C
    import GaudiConfig2.Configurables.Gaudi.Tests.Histograms.AxesLabels as T

    algs = []
    svcs = []

    HistoAlgo = T.HistWithLabelsAlg
    algs.append(HistoAlgo("HistoAlgo"))

    svcs.append(C.Gaudi.Histograming.Sink.Root(FileName=FILENAME))
    svcs.append(C.Gaudi.Monitoring.JSONSink(FileName=FILENAMEJSON))
    svcs.append(C.Gaudi.Monitoring.MessageSvcSink())

    yield from algs
    yield from svcs

    yield C.ApplicationMgr(
        EvtMax=5,
        EvtSel="NONE",
        TopAlg=algs,
        ExtSvc=svcs,
    )


class TestAxesLabels(GaudiExeTest):
    currentFile = inspect.getfile(inspect.currentframe())
    command = ["gaudirun.py", f"{currentFile}:config"]

    def test_axis_labels(self, cwd):
        assert os.path.exists(cwd / FILENAME)

        import ROOT

        f = ROOT.TFile.Open(str(cwd / FILENAME))
        h = f.Get("HistoAlgo/hist")
        axis = h.GetXaxis()
        labels = list(axis.GetLabels())
        expected = ["a", "b", "c", "d", "e"]
        assert labels == expected

    def validate_json_with_reference(self, cwd):
        self.validate_json_with_reference(
            cwd / FILENAMEJSON, "../refs/histograms/axes_labels.json"
        )

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

FILENAME = "directories.root"
FILENAMEJSON = "directories.json"


def config():
    import GaudiConfig2.Configurables as C
    import GaudiConfig2.Configurables.Gaudi.Tests.Histograms.Directories as D

    algs = []
    tools = []
    svcs = []

    Alg = D.HistoGroupsAlg
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


class TestDirectories(GaudiExeTest):
    currentFile = inspect.getfile(inspect.currentframe())
    command = ["gaudirun.py", f"{currentFile}:config"]

    def test_axis_labels(self, cwd):
        assert os.path.exists(cwd / FILENAME)

        import ROOT

        f = ROOT.TFile.Open(str(cwd / FILENAME))
        for name in [
            f"{component}/{histogram}"
            for component in ["Alg", "Alg/Tool"]
            for histogram in [
                "Top",
                "Group/First",
                "Group/Second",
                "Group/SubGroup/Third",
            ]
        ]:
            h = f.Get(name)
            assert h, f"missing histogram {name}"

    def validate_json_with_reference(self, cwd):
        self.validate_json_with_reference(
            cwd / FILENAMEJSON, "../refs/histograms/directories.json"
        )

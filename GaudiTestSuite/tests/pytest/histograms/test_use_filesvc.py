#####################################################################################
# (c) Copyright 2025 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from pathlib import Path

from GaudiTesting import GaudiExeTest

FILENAME = f"{Path(__file__).stem}.root"


def config():
    import GaudiConfig2.Configurables as C
    from GaudiConfig2.Configurables.Gaudi.TestSuite.Counter import (
        GaudiRootHistoAlgorithm as Alg,
    )

    algs = []
    tools = []
    svcs = []

    algs.append(
        Alg(
            "Alg",
            CustomGauss_Title="Gaussian mean=0, sigma=1, atomic",
            CustomGauss_Axis0=(100, -5, 5, "X"),
            CustomGaussFlat_Title="Gaussian V Flat, atomic",
            CustomGaussFlat_Axis0=(50, -5, 5, "X"),
            CustomGaussFlat_Axis1=(50, -5, 5, "Y"),
            CustomGaussFlatGauss_Title="Gaussian V Flat V Gaussian, atomic",
            CustomGaussFlatGauss_Axis0=(10, -5, 5, "X"),
            CustomGaussFlatGauss_Axis1=(10, -5, 5, "Y"),
            CustomGaussFlatGauss_Axis2=(10, -5, 5, "Z"),
        )
    )
    svcs.append(
        C.FileSvc(
            Config={
                "histo_file": f"{FILENAME}?mode=recreate",
            }
        )
    )
    svcs.append(C.Gaudi.Histograming.Sink.Root(FileName="histo_file"))

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
    command = ["gaudirun.py", f"{__file__}:config"]

    def test_root_file(self, cwd):
        root_file = cwd / FILENAME
        assert root_file.exists()

        import ROOT

        f = ROOT.TFile.Open(str(root_file))
        for name in [
            f"Alg/{histogram}"
            for histogram in [
                "CustomGauss",
                "CustomGaussFlat",
                "CustomGaussFlatGauss",
                "Gauss",
                "GaussBuf",
                "GaussFlat",
                "GaussFlatBuf",
                "GaussFlatGauss",
                "GaussFlatGaussBuf",
            ]
        ]:
            h = f.Get(name)
            assert h, f"missing histogram {name}"

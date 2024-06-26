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
from pathlib import Path

import pytest
from GaudiTesting import GaudiExeTest


@pytest.fixture(scope="class")
def timing_histograms(cwd: Path):
    """
    Get the expected TDirectory in the ROOT file as a fixture
    to be able to parametrize the test.
    """
    import ROOT

    filename = str(cwd / "timing_histos.root")
    f = ROOT.TFile.Open(filename)
    assert f, f"Cannot open file {filename}"

    toolname = "TIMER.TIMER"
    d = f.Get(toolname)
    assert d, f"Missing directory {toolname}"

    yield d


EXPECTED_LABELS = [
    "EVENT LOOP                    ",
    " ParentAlg                    ",
    "  SubAlg1                     ",
    "  SubAlg2                     ",
    " StopperAlg                   ",
    " TopSequence                  ",
    "  Sequence1                   ",
    "   Prescaler1                 ",
    "   HelloWorld                 ",
    "   Counter1                   ",
    "  Sequence2                   ",
    "   Prescaler2                 ",
    "   Counter2                   ",
    " ANDSequence                  ",
    "  AND                         ",
    "  ANDCounter                  ",
    " ORSequence                   ",
    "  OR                          ",
    "  ORCounter                   ",
]


class Test(GaudiExeTest):
    command = ["gaudirun.py", "../qmtest/options/TimingHistograms.py"]
    reference = "refs/TimingHistograms.yaml"

    @pytest.mark.parametrize("histogram", ["CPUTime", "ElapsedTime", "Count"])
    def test_root_file(self, histogram, timing_histograms):
        "Check that the ROOT file contains the expected histograms"
        h = timing_histograms.Get(histogram)
        assert h, f"Missing histogram {histogram!r}"
        labels = list(h.GetXaxis().GetLabels())
        assert labels == EXPECTED_LABELS

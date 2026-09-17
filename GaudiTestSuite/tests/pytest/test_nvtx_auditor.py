#####################################################################################
# (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from collections import Counter

from GaudiTesting import GaudiExeTest


class TestNVTXAudit(GaudiExeTest):
    output = "NVTXAudit"
    command = [
        "nsys",
        "profile",
        "--trace=nvtx",
        "--export",
        "text",
        "-o",
        output,
        "gaudirun.py",
        "../../../GaudiCUDA/tests/options/NVTXAudit.py",
    ]

    def test_written_ranges(self, cwd):
        ranges = Counter()

        with (cwd / f"{self.output}.txt").open() as f:
            for line in f:
                line = line.strip()
                if line.startswith("Text:"):
                    ranges[line] += 1

        number_of_events = 4

        for alg in ["Sequencer", "InnerAlg1", "InnerAlg2", "OuterAlg"]:
            assert ranges[f'Text: "{alg}:Start"'] == 1
            assert ranges[f'Text: "{alg}:Initialize"'] == 1
            assert ranges[f'Text: "{alg}:Execute"'] == number_of_events
            assert ranges[f'Text: "{alg}:Finalize"'] == 1
            assert ranges[f'Text: "{alg}:Stop"'] == 1

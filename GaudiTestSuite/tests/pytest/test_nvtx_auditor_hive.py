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


class TestNVTXAuditHive(GaudiExeTest):
    output = "NVTXAuditHive"
    command = [
        "nsys",
        "profile",
        "--trace=nvtx",
        "--export",
        "text",
        "-o",
        output,
        "gaudirun.py",
        "../../../GaudiCUDA/tests/options/NVTXAuditHive.py",
    ]

    def test_written_ranges(self, cwd):
        ranges = Counter()

        with (cwd / f"{self.output}.txt").open() as f:
            for line in f:
                line = line.strip()
                if line.startswith("Text:"):
                    ranges[line] += 1

        for alg in ["GPUAlg1", "GPUAlg2", "CPUAlg1", "GPUAlg2"]:
            assert ranges[f'Text: "{alg}:Start"'] == 1
            assert ranges[f'Text: "{alg}:Initialize"'] == 1
            assert ranges[f'Text: "{alg}:Finalize"'] == 1
            assert ranges[f'Text: "{alg}:Stop"'] == 1

        number_of_events = 4
        # synchronous CPU algorithms should have only one execution range per event
        assert ranges['Text: "CPUAlg1:Execute"'] == number_of_events
        assert ranges['Text: "CPUAlg2:Execute"'] == number_of_events
        # asynchronous GPU algorithms can have multiple execution ranges per event
        # the GPUCruncher algorithm is designed to have at least one suspension per event, so more ranges are expected
        assert ranges['Text: "GPUAlg1:Execute"'] > number_of_events
        assert ranges['Text: "GPUAlg2:Execute"'] > number_of_events

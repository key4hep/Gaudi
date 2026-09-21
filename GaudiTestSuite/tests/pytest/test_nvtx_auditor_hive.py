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
import csv
import subprocess

from GaudiTesting import GaudiExeTest


class TestNVTXAudit(GaudiExeTest):
    output = "NVTXAuditHive"
    command = [
        "nsys",
        "profile",
        "--trace=nvtx",
        "-o",
        output,
        "gaudirun.py",
        "../../../GaudiCUDA/tests/options/NVTXAuditHive.py",
    ]

    def test_written_ranges(self, cwd):
        stats_command = [
            "nsys",
            "stats",
            f"{self.output}.nsys-rep",
            "--report=nvtx_sum",
            "--format=csv",
            f"--output={self.output}",
        ]
        subprocess.run(stats_command, cwd=cwd, check=True)
        ranges = {}

        with (cwd / f"{self.output}_nvtx_sum.csv").open(newline="") as f:
            reader = csv.DictReader(f)
            for row in reader:
                ranges[row["Range"]] = int(row["Instances"])

        for alg in ["GPUAlg1", "GPUAlg2", "CPUAlg1", "GPUAlg2"]:
            assert ranges[f"Gaudi:{alg}:Start"] == 1
            assert ranges[f"Gaudi:{alg}:Initialize"] == 1
            assert ranges[f"Gaudi:{alg}:Finalize"] == 1
            assert ranges[f"Gaudi:{alg}:Stop"] == 1

        number_of_events = 4
        # synchronous CPU algorithms should have only one execution range per event
        assert ranges["Gaudi:CPUAlg1:Execute"] == number_of_events
        assert ranges["Gaudi:CPUAlg2:Execute"] == number_of_events
        # asynchronous GPU algorithms can have multiple execution ranges per event
        # the GPUCruncher algorithm is designed to have at least one suspension per event, so more ranges are expected
        assert ranges["Gaudi:GPUAlg1:Execute"] > number_of_events
        assert ranges["Gaudi:GPUAlg2:Execute"] > number_of_events

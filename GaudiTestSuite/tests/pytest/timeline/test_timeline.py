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
import yaml
from GaudiTesting import GaudiExeTest
from GaudiTesting.preprocessors import RegexpReplacer, normalizeTestSuite


class TestTimeline(GaudiExeTest):
    command = ["gaudirun.py", "../../../options/Timeline.py"]
    reference = "../refs/Timeline.yaml"

    def replacer(inp):
        # since time and time difference will be different each time we test
        # let's at least check that the numbers we expect to get are all non zero
        def zeroIsErr(x):
            return "-" if int(x) > 0 else "err"

        return (
            zeroIsErr(inp.group(1))
            + " "
            + zeroIsErr(inp.group(2))
            + " "
            + inp.group(3)
            + " "
            + zeroIsErr(inp.group(4))
        )

    preprocessor = normalizeTestSuite + RegexpReplacer(
        r"^([0-9]+) ([0-9]+) ([^ ]+) ([0-9]+)", replacer
    )

    def test_stdout(self, monkeypatch):
        monkeypatch.chdir(self.cwd)
        with open(self.resolve_path(self.reference), "r") as ref_file:
            data = yaml.safe_load(ref_file)
        assert self.preprocessor(open("myTimeline.csv").read()) == self.preprocessor(
            data["stdout"]
        )

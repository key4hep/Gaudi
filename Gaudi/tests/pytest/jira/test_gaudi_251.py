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
from conftest import OptParseTest


class Test(OptParseTest):
    """
    https://its.cern.ch/jira/browse/GAUDI-251
    """

    def options(self):
        from Gaudi.Configuration import ConfigurableUser

        class TestAlg(ConfigurableUser):
            __slots__ = {"Dict": {"a": 1}, "List": ["a", "b"]}

            def getGaudiType(self):
                return "Test"  # To avoid skipping of ConfigurableUser

        alg1 = TestAlg("Test1", Dict={"b": 10}, List=["A", "B"])

        alg2 = alg1.clone("Test2")
        alg2.Dict["a"] = 20
        alg2.List.append("C")

    expected_dump = {
        "Test1": {"Dict": {"b": 10}, "List": ["A", "B"]},
        "Test2": {"Dict": {"a": 20, "b": 10}, "List": ["A", "B", "C"]},
    }

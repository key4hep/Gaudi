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

from GaudiTesting import GaudiExeTest


class TestHistoProps(GaudiExeTest):
    command = ["gaudirun.py", "-v", "--dry-run"]

    def options(self):
        from Configurables import Gaudi__TestSuite__HistoProps

        h = Gaudi__TestSuite__HistoProps()
        h.Histo1 = ("MyHistogram", -1, 1, 100)
        h.Histo1 = ("MyHistogram", -5, 5, 500)

    @classmethod
    def update_env(cls, env):
        cls.unset_vars(env, ["GAUDIAPPNAME", "GAUDIAPPVERSION"])

    def test_configuration(self, stdout):
        conf_dict = eval("\n".join(stdout.decode().splitlines()))
        expected = {
            "Gaudi::TestSuite::HistoProps": {"Histo1": ("MyHistogram", -5, 5, 500)}
        }

        assert conf_dict == expected

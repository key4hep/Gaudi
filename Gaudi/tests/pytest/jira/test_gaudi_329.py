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

from GaudiTesting import GaudiExeTest


class Test(GaudiExeTest):
    """
    https://its.cern.ch/jira/browse/GAUDI-329
    """

    command = ["gaudirun.py", "-v", "-n"]
    environment = [
        f"PYTHONPATH={Path(__file__).parent.parent.parent/'python'}:$PYTHONPATH"
    ]

    def options(self):
        # Add the fake configurables to the path
        # only the _merge_confDb.py in InstallArea/python/* are used automatically
        import Test_confDb  # noqa: F401

        # ------------------------------------------------------------------------------\
        # https://savannah.cern.ch/bugs/?61144
        from Configurables import MyTestTool

        print(MyTestTool())
        # ------------------------------------------------------------------------------/

    def test_stderr(self, stderr):
        # require empty stderr
        assert not stderr.strip()

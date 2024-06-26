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

from conftest import OptParseTest


class TestGaudi210(OptParseTest):
    """
    https://its.cern.ch/jira/browse/GAUDI-210
    """

    environment = [
        f"OPTSROOT={Path(__file__).parent.parent.parent/'pyjobopts'}",
    ]

    def options(self):
        import os
        from pathlib import Path

        from Gaudi.Configuration import importOptions

        # $PWD is set to the directory pytest is started from
        root = Path(os.environ["OPTSROOT"])
        importOptions(str(root / "gaudi_210.py"))
        importOptions(str(root / "subdir" / "gaudi_210.py"))

    expected_dump = {"myAlg": {"Opt1": 1, "Opt2": 2}}

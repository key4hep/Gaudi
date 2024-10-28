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


def config():
    """
    Configure a job writing to an empty file.
    """
    from Configurables import GaudiPersistency
    from GaudiConfig2 import Configurables as C

    GaudiPersistency()

    out = C.OutputStream(
        ItemList=["/Event#999"],
    )
    app = C.ApplicationMgr(EvtMax=1, OutStream=[out.__opt_value__()])

    return [app, out]


class Test(GaudiExeTest):
    """
    Check that an output file is created even if no event is written to it.
    """

    command = ["gaudirun.py", f"{__file__}:config"]
    returncode = 1

    def test_stdout(self, stdout):
        assert b"OutputStream instances require an output" in stdout

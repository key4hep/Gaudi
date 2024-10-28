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
import os

from GaudiTesting import GaudiExeTest

OUTPUT_FILE_NAME = "empty-output-file.dst"


def config():
    """
    Configure a job writing to an empty file.
    """
    from Configurables import GaudiPersistency
    from GaudiConfig2 import Configurables as C

    GaudiPersistency()

    out = C.OutputStream(
        ItemList=["/Event#999"],
        Output=f"DATAFILE='PFN:{OUTPUT_FILE_NAME}' SVC='Gaudi::RootCnvSvc' OPT='RECREATE'",
    )
    app = C.ApplicationMgr(EvtMax=0, OutStream=[out.__opt_value__()])

    return [app, out]


class Test(GaudiExeTest):
    """
    Check that an output file is created even if no event is written to it.
    """

    command = ["gaudirun.py", f"{__file__}:config"]

    def test_output_file(self, cwd):
        assert os.path.exists(cwd / OUTPUT_FILE_NAME)

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
import sys

from GaudiTests import run_gaudi


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


def test(tmp_path):
    """
    Check that an output file is created even if no event is written to it.
    """
    os.chdir(tmp_path)

    result = run_gaudi(
        f"{__file__}:config", capture_output=True, errors="surrogateescape"
    )
    print(result.stdout, end="")
    print(result.stderr, end="", file=sys.stderr)

    assert result.returncode == 1
    assert "OutputStream instances require an output" in result.stdout

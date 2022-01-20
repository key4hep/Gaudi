#####################################################################################
# (c) Copyright 2022 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import json
import os

from GaudiTests import run_gaudi


def config():
    from Configurables import ApplicationMgr
    from GaudiConfig2 import Configurables as C

    ApplicationMgr(AppName="Test", EvtMax=10)
    app = C.ApplicationMgr(AppName="Test", EvtMax=10)
    return [app]


def test(tmp_path):
    opts_dump = tmp_path / "opts.json"

    run_gaudi(f"{__file__}:config", "-n", "-o", opts_dump, check=True)

    assert opts_dump.exists()
    opts = json.load(opts_dump.open())
    assert opts["ApplicationMgr.AppName"] == '"Test"'
    assert opts["ApplicationMgr.EvtMax"] == "10"

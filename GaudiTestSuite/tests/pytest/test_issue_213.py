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

from GaudiTests import run_gaudi


def config():
    """
    Mix GaudiConfig2 and legacy ConfigurableUser configurations.
    """
    # use a ConfigurableUser specialization
    from Configurables import GaudiTestSuiteCommonConf

    GaudiTestSuiteCommonConf(DummyEvents=42)

    # no GaudiConfig2 configurables to add


def test(tmp_path):
    """
    Run gaudirun.py with a Python option function that instantiates
    a ConfigurableUser specialization.

    See https://gitlab.cern.ch/gaudi/Gaudi/-/issues/213
    """
    opts_dump = tmp_path / "opts.json"

    run_gaudi(f"{__file__}:config", "-n", "-o", opts_dump, check=True)

    assert opts_dump.exists()
    opts = json.load(opts_dump.open())

    assert opts["ApplicationMgr.EvtMax"] == "42"

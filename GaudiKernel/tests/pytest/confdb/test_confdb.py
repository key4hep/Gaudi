#####################################################################################
# (c) Copyright 1998-2026 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import re
from os.path import join

from common import data_root

import GaudiPluginService.cpluginsvc

GAUDI_DEFAULT_PLUGIN_PATH = list(
    GaudiPluginService.cpluginsvc.GAUDI_DEFAULT_PLUGIN_PATH
)


def test_bad(monkeypatch, caplog):
    """
    test the failure when loading malformed data
    """
    monkeypatch.setattr(
        GaudiPluginService.cpluginsvc,
        "GAUDI_DEFAULT_PLUGIN_PATH",
        [join(data_root, "bad")] + GAUDI_DEFAULT_PLUGIN_PATH,
    )

    from GaudiKernel.ConfigurableDb import loadConfigurableDb

    loadConfigurableDb()

    warnings = [rec.message for rec in caplog.records if rec.levelname == "WARNING"]
    assert [
        record for record in warnings if re.match(r"Could not load.*bad.confdb", record)
    ]
    assert [
        record
        for record in warnings
        if re.match(r"Reason: invalid line format", record)
    ]


def test_regular(monkeypatch):
    """
    test loading of .confdb files
    """
    monkeypatch.setattr(
        GaudiPluginService.cpluginsvc,
        "GAUDI_DEFAULT_PLUGIN_PATH",
        [join(data_root, "regular", n) for n in ["dir1", "dir2"]]
        + GAUDI_DEFAULT_PLUGIN_PATH,
    )

    from GaudiKernel.ConfigurableDb import cfgDb, loadConfigurableDb

    loadConfigurableDb()
    for c in [t + n for t in ["Alg", "Svc"] for n in ["1a", "1b", "2"]]:
        assert c in cfgDb, "missing entry for " + c


def test_merged(monkeypatch):
    """
    test priority of *_merged.confdb files over *.confdb
    """
    monkeypatch.setattr(
        GaudiPluginService.cpluginsvc,
        "GAUDI_DEFAULT_PLUGIN_PATH",
        [join(data_root, "merged", n) for n in ["dir1", "dir2"]]
        + GAUDI_DEFAULT_PLUGIN_PATH,
    )

    from GaudiKernel.ConfigurableDb import cfgDb, loadConfigurableDb

    loadConfigurableDb()
    for c in [t + n for t in ["Alg", "Svc"] for n in ["M", "2"]]:
        assert c in cfgDb, "missing entry for " + c
    for c in ["Alg1", "Svc1"]:
        assert c not in cfgDb, "unwanted entry for " + c

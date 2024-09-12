#####################################################################################
# (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from pathlib import Path

from Gaudi.Configuration import configurationDict, importOptions
from GaudiKernel.Configurable import purge


def test(monkeypatch):
    monkeypatch.setenv(
        "JOBOPTSEARCHPATH", str(Path(__file__).parent.parent / "pyjobopts")
    )

    option_file = "test_purge1.py"
    expected = {"AuditorSvc": {"Auditors": ["ChronoAuditor", "AlgTimingAuditor/TIMER"]}}

    importOptions(option_file)
    first = configurationDict()
    assert first == expected

    purge()
    purged = configurationDict()
    assert purged == {}

    importOptions(option_file)
    second = configurationDict()
    assert second == expected

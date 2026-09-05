#####################################################################################
# (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import pytest
from GaudiTesting import GaudiExeTest


@pytest.mark.ctest_fixture_setup("ntuple_diskbuffer_columnar")
@pytest.mark.shared_cwd("ntuple_diskbuffer")
class Test(GaudiExeTest):
    """Stock column-wise writing; the reference the buffered run is compared to."""

    command = [
        "gaudirun.py",
        "../../../options/NTupleDiskBuffer.py",
        "--option",
        "NTupleSvc().Output = [\"MyTuples DATAFILE='ntuple_columnar.root' OPT='NEW' TYP='ROOT'\"]",
    ]

    def test_no_buffer(self, stdout, cwd):
        assert b"Buffering TTree" not in stdout
        assert not list(cwd.glob("*.ntbuf"))

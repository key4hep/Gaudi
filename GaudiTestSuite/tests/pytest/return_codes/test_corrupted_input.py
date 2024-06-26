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
import pytest
from GaudiTesting import GaudiExeTest


@pytest.mark.ctest_fixture_required("root_io_base")
@pytest.mark.shared_cwd("root_io")
class Test(GaudiExeTest):
    command = ["gaudirun.py", "-v", "../../../options/ROOT_IO/Read.py"]

    def options(self):
        # produce a corrupted (truncated) copy of the output of ROOT_IO/Write.py
        import shutil

        from Gaudi.Configuration import (
            EventSelector,
        )

        shutil.copy("ROOTIO.dst", "ROOTIO_corrupted.dst")

        # find a specific byte sequence to determine where to corrupt the file
        magic_bytes = b"_Event_Collision_0_MyVertices."
        with open("ROOTIO_corrupted.dst", "rb") as f:
            data = f.read()
            if data.count(magic_bytes) != 1:
                raise NotImplementedError("The test file is not as expected")
            seek_index = data.index(magic_bytes) + 735

        with open("ROOTIO_corrupted.dst", "rb+") as f:
            f.seek(seek_index)
            f.write(b"corruption!")

        esel = EventSelector()
        esel.Input = [
            "DATAFILE='PFN:ROOTIO_corrupted.dst' SVC='Gaudi::RootEvtSelector' OPT='READ'"
        ]

    returncode = 16

    def test_stdout(self, stdout):
        assert (
            b"ApplicationMgr      ERROR Application Manager Terminated with error code 16"
            in stdout
        )

    def test_stderr(self):
        pass

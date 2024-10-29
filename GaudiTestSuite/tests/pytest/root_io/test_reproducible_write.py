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
import filecmp

import pytest
from GaudiTesting import GaudiExeTest


@pytest.mark.ctest_fixture_required("root_io_base")
@pytest.mark.shared_cwd("root_io")
class Test(GaudiExeTest):
    command = ["gaudirun.py", "-v", "../../../options/ROOT_IO/Write.py"]
    reference = "../refs/ROOT_IO/Write.yaml"
    environment = ["GAUDIAPPNAME=", "GAUDIAPPVERSION="]

    def options(self):
        import os
        import shutil

        try:
            os.mkdir("ReproducibleWrite")
        except FileExistsError:
            pass
        os.chdir("ReproducibleWrite")
        shutil.copy("../ROOTIO.xml", "ROOTIO.xml")

    @pytest.mark.parametrize("name", ["ROOTIO.dst", "ROOTIO.mdst"])
    def test_output_content(self, name, cwd):
        assert filecmp.cmp(cwd / name, cwd / "ReproducibleWrite" / name, shallow=False)

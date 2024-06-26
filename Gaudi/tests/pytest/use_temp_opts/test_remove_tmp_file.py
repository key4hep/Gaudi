#!/usr/bin/env python3
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
import os

from conftest import OptParseTest


class Test(OptParseTest):
    @classmethod
    def update_env(cls, env):
        # The environment variable GAUDI_TEMP_OPTS_FILE is used
        # internally by gaudirun.py --use-temp-opts
        tmp_opts = os.path.join(cls.popen_kwargs["cwd"] / "tmp.opts")
        with open(tmp_opts, "w") as f:
            f.write("MyAlg.MyOpt = 1;\n")
        env["GAUDI_TEMP_OPTS_FILE"] = tmp_opts

    options = None  # options are taken from $GAUDI_TEMP_OPTS_FILE

    expected_dump = {"MyAlg": {"MyOpt": 1}}

    def test_tmp_file_removed(self):
        assert not os.path.exists(self.cwd / "tmp.opts")

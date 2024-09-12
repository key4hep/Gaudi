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
from GaudiTesting import GaudiExeTest


class TestCompatAliases(GaudiExeTest):
    command = ["../../scripts/test_compat_aliases.py"]

    @classmethod
    def update_env(cls, env):
        env["LD_LIBRARY_PATH"] = cls.expand_vars_from(
            "$ENV_CMAKE_BINARY_DIR/GaudiPluginService:$ENV_CMAKE_BINARY_DIR/Gaudi/GaudiPluginService:$LD_LIBRARY_PATH",
            env,
        )

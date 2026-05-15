#####################################################################################
# (c) Copyright 2024-2026 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiTesting import GaudiExeTest


class TestGaudiPluginPath(GaudiExeTest):
    command = ["gaudirun.py", "../../options/CounterAlg.py"]
    reference = "refs/CounterAlg.yaml"

    @classmethod
    def update_env(cls, env):
        plugin_path = env["GAUDI_PLUGIN_PATH"].split(":")
        env["LD_LIBRARY_PATH"] = ":".join(
            path
            for path in env.get("LD_LIBRARY_PATH", "").split(":")
            # ensure the plugin path directories are not in LD_LIBRARY_PATH
            if path not in plugin_path
            # but we need to keep the GaudiKernel directory
            or "GaudiKernel" in path
        )

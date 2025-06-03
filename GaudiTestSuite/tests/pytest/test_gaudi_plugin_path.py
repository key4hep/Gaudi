#####################################################################################
# (c) Copyright 2024-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import os

from GaudiTesting import GaudiExeTest


class TestGaudiPluginPath(GaudiExeTest):
    command = ["gaudirun.py", "../../options/CounterAlg.py"]
    reference = "refs/CounterAlg.yaml"

    @classmethod
    def update_env(cls, env):
        env["GAUDI_PLUGIN_PATH"] = cls.expand_vars_from(
            "$ENV_CMAKE_BINARY_DIR/GaudiPluginService:$ENV_CMAKE_BINARY_DIR/Gaudi/GaudiPluginService:$LD_LIBRARY_PATH",
            env,
        )

        # libGaudiKernel is loaded in __init__.py so it needs to be in LD_LIBRARY_PATH
        # Try to also add the location of libstdc++ or libc++, searching only one level
        # deep since a recursive search is expensive (for example, on the LCG stacks)
        original_ld_library_path = cls.expand_vars_from("$LD_LIBRARY_PATH", env)
        to_add = []
        for path in original_ld_library_path.split(":"):
            if os.path.exists(path):
                for file in os.listdir(path):
                    if file.startswith("libstdc++") or file.startswith("libc++"):
                        to_add.append(path)
                        break
        env["LD_LIBRARY_PATH"] = cls.expand_vars_from(
            f"$ENV_CMAKE_BINARY_DIR/GaudiKernel:{':'.join(to_add)}", env
        )

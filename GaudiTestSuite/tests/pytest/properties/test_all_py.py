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
from GaudiTesting.preprocessors import RegexpReplacer, normalizeTestSuite


class TestAllPy(GaudiExeTest):
    command = ["gaudirun.py", "../../../options/Properties.py"]
    reference = "../refs/Properties_py.yaml"

    @classmethod
    def update_env(cls, env):
        env["JOBOPTSEARCHPATH"] = cls.expand_vars_from(
            "$ENV_PROJECT_SOURCE_DIR/GaudiTestSuite/options:$ENV_PROJECT_SOURCE_DIR/Gaudi/GaudiTestSuite/options",
            env,
        )
        cls.unset_vars(env, ["GAUDIAPPNAME", "GAUDIAPPVERSION"])

    preprocessor = normalizeTestSuite + RegexpReplacer(
        r"(JobOptionsSvc\.(SEARCH)?PATH|ApplicationMgr\.JobOptionsPath): '.*'",
        r"\1: '...'",
    )

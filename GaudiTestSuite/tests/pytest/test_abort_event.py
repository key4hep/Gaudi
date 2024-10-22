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


class TestAbortEvent(GaudiExeTest):
    command = ["gaudirun.py"]

    @classmethod
    def update_env(cls, env):
        env["STDOPTS"] = cls.expand_vars_from(
            "$ENV_PROJECT_SOURCE_DIR/GaudiTestSuite/options:$ENV_PROJECT_SOURCE_DIR/Gaudi/GaudiTestSuite/options",
            env,
        )
        env["JOBOPTSEARCHPATH"] = cls.expand_vars_from(
            "$ENV_PROJECT_SOURCE_DIR/GaudiTestSuite/tests/qmtest:$ENV_PROJECT_SOURCE_DIR/GaudiTestSuite/options:$ENV_PROJECT_SOURCE_DIR/Gaudi/tests/pyjobopts:$ENV_PROJECT_SOURCE_DIR/Gaudi/tests:$ENV_PROJECT_SOURCE_DIR/Gaudi/GaudiTestSuite/tests/qmtest:$ENV_PROJECT_SOURCE_DIR/Gaudi/GaudiTestSuite/options:$ENV_PROJECT_SOURCE_DIR/Gaudi/Gaudi/tests/pyjobopts:$ENV_PROJECT_SOURCE_DIR/Gaudi/Gaudi/tests",
            env,
        )
        env["PYTHONPATH"] = cls.expand_vars_from(
            "$ENV_PROJECT_SOURCE_DIR/GaudiTestSuite/tests/qmtest:$ENV_PROJECT_SOURCE_DIR/Gaudi/tests/python:$ENV_PROJECT_SOURCE_DIR/Gaudi/GaudiTestSuite/tests/qmtest:$ENV_PROJECT_SOURCE_DIR/Gaudi/Gaudi/tests/python:$PYTHONPATH",
            env,
        )

    def options(self):
        from Configurables import AbortEventAlg, HelloWorld

        from Gaudi.Configuration import ApplicationMgr, MessageSvc, importOptions

        importOptions("Common.opts")

        h1 = HelloWorld("HelloWorld1")
        h2 = HelloWorld("HelloWorld2")

        alg = AbortEventAlg(AbortedEventNumber=3)

        app = ApplicationMgr(TopAlg=[h1, alg, h2], EvtSel="NONE", EvtMax=5)  # noqa: F841

        MessageSvc().setDebug.append("EventLoopMgr")

    def filter_out(stdout):
        return "\n".join(
            line
            for line in stdout.splitlines()
            if "EventLoopMgr      SUCCESS Event Number =" not in line
            and " ---> Loop Finished" not in line
        )

    test_check_line = GaudiExeTest.find_reference_block(
        """
        HelloWorld1          INFO executing....
        HelloWorld2          INFO executing....
        HelloWorld1          INFO executing....
        HelloWorld2          INFO executing....
        HelloWorld1          INFO executing....
        EventLoopMgr        DEBUG AbortEvent incident fired by AbortEventAlg
        HelloWorld1          INFO executing....
        HelloWorld2          INFO executing....
        HelloWorld1          INFO executing....
        HelloWorld2          INFO executing....
        """,
        preprocessor=filter_out,
    )

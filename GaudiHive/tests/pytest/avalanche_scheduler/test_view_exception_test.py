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


class Test(GaudiExeTest):
    command = ["gaudirun.py", "-v", "../../../options/SubSlotException.py"]

    @classmethod
    def update_env(cls, env):
        # this is to prevent GaudiException to print a stack trace (it confuses the test)
        if "ENABLE_BACKTRACE" in env:
            del env["ENABLE_BACKTRACE"]

    timeout = 120
    reference = "../refs/ViewExceptionTest.yaml"
    returncode = 6

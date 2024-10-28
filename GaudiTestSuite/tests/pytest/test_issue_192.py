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


def config():
    """
    Dummy no-op configuration function
    """
    return []


class Test(GaudiExeTest):
    """
    Run gaudirun.py with full path to a Python module.

    See https://gitlab.cern.ch/gaudi/Gaudi/-/issues/192
    """

    command = ["gaudirun.py", f"{__file__}:config", "-n"]

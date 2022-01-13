#####################################################################################
# (c) Copyright 2022 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiTests import run_gaudi


def config():
    """
    Dummy no-op configuration function
    """
    return []


def test():
    """
    Run gaudirun.py with full path to a Python module.

    See https://gitlab.cern.ch/gaudi/Gaudi/-/issues/192
    """
    run_gaudi(f"{__file__}:config", "-n", check=True)

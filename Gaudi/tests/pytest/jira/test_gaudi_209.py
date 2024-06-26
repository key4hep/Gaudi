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
from conftest import OptParseTest


class TestGaudi209(OptParseTest):
    """
    https://its.cern.ch/jira/browse/GAUDI-209
    """

    options = """
    EventSelector.Input = {
        "DATAFILE='PFN:dcap://blah.blah/foo/bar/file.dst' TYPE='POOL_ROOTTREE' OPT='READ'"
    };
    """

    expected_dump = {
        "EventSelector": {
            "Input": [
                "DATAFILE='PFN:dcap://blah.blah/foo/bar/file.dst' TYPE='POOL_ROOTTREE' OPT='READ'"
            ]
        }
    }

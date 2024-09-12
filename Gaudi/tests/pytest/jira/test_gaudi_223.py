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


class Test(OptParseTest):
    """
    https://its.cern.ch/jira/browse/GAUDI-223
    """

    environment = [
        "VAR1=v1",
        "VAR2=v2",
    ]

    options = """
    Alg.Prop1 = "$VAR1";
    Alg.Prop2 = { "$VAR1", "$VAR2" };
    Alg.Prop3 = ( "$VAR1", "$VAR2" );
    Alg.Prop4 = { "$VAR1": "value", "key": "$VAR2" };
    """

    expected_dump = {
        "Alg": {
            "Prop1": "$VAR1",
            "Prop2": ["$VAR1", "$VAR2"],
            "Prop3": ("$VAR1", "$VAR2"),
            "Prop4": {"$VAR1": "value", "key": "$VAR2"},
        }
    }

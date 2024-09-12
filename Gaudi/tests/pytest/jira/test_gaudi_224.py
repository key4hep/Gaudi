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
    https://its.cern.ch/jira/browse/GAUDI-224
    """

    options = """
    Alg.Prop = { "a" };
    Alg.Prop = @Alg.RefProp;
    Alg.RefProp = { "b" };
    """

    expected_dump = {"Alg": {"Prop": ["b"], "RefProp": ["b"]}}

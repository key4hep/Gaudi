#####################################################################################
# (c) Copyright 2025 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from Gaudi.Main import parseOpt, toOpt


def test_toOpt():
    assert toOpt('some "text"') == '"some \\"text\\""'
    assert toOpt("first\nsecond") == '"first\nsecond"'
    assert toOpt({"a": [1, 2, "3"]}) == '{"a": [1, 2, "3"]}'


def test_parseOpt():
    assert parseOpt("123") == 123
    assert parseOpt('"some\n\\"text\\""') == 'some\n"text"'
    assert parseOpt("") == ""

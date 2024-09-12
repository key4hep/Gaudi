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


class TestStringKeyEx(GaudiExeTest):
    command = ["../../scripts/StringKeyEx.py"]

    testBlock1 = GaudiExeTest.find_reference_block(
        """
        StringKeys        SUCCESS check for       StringKey 'key'
        StringKeys        SUCCESS  In Map 1: True
        StringKeys        SUCCESS  In Map 2: True
        StringKeys        SUCCESS  In Map 3: True
        StringKeys        SUCCESS  In Map 4: True
        StringKeys        SUCCESS  In Map01: True
        StringKeys        SUCCESS  In Map02: True
        StringKeys        SUCCESS  In Map03: True
        StringKeys        SUCCESS  In Map04: True
        StringKeys        SUCCESS check for std::string key 'rrr'
        StringKeys        SUCCESS  In Map 1: False
        StringKeys        SUCCESS  In Map 2: False
        StringKeys        SUCCESS  In Map 3: False
        StringKeys        SUCCESS  In Map 4: False
        StringKeys        SUCCESS  In Map01: False
        StringKeys        SUCCESS  In Map02: False
        StringKeys        SUCCESS  In Map03: False
        StringKeys        SUCCESS  In Map04: False
        """
    )

    testBlock2 = GaudiExeTest.find_reference_block(
        """
        StringKeys        SUCCESS check for       StringKey 'new Key'
        StringKeys        SUCCESS  In Map 1: True
        StringKeys        SUCCESS  In Map 2: True
        StringKeys        SUCCESS  In Map 3: True
        StringKeys        SUCCESS  In Map 4: True
        StringKeys        SUCCESS  In Map01: True
        StringKeys        SUCCESS  In Map02: True
        StringKeys        SUCCESS  In Map03: True
        StringKeys        SUCCESS  In Map04: True
        StringKeys        SUCCESS check for std::string key 'rrr'
        StringKeys        SUCCESS  In Map 1: True
        StringKeys        SUCCESS  In Map 2: True
        StringKeys        SUCCESS  In Map 3: True
        StringKeys        SUCCESS  In Map 4: True
        StringKeys        SUCCESS  In Map01: True
        StringKeys        SUCCESS  In Map02: True
        StringKeys        SUCCESS  In Map03: True
        StringKeys        SUCCESS  In Map04: True
        """
    )

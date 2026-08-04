#####################################################################################
# (c) Copyright 2026 CERN for the benefit of the LHCb and ATLAS collaborations       #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization or     #
# submit itself to any jurisdiction.                                                #
#####################################################################################
import cppyy

from GaudiKernel import Constants
from GaudiKernel.Constants import MessageLevel


def test_message_levels_match_cpp():
    cppyy.include("GaudiKernel/IMessageSvc.h")

    for level in MessageLevel:
        assert int(level) == int(getattr(cppyy.gbl.MSG, level.name))
    assert len(MessageLevel) == cppyy.gbl.MSG.NUM_LEVELS


def test_message_level_is_reexported():
    from Gaudi.Configuration import MessageLevel as exported_message_level

    assert exported_message_level is MessageLevel


def test_message_level_constants_remain_integers():
    assert Constants.ALL == MessageLevel.NIL
    assert isinstance(Constants.ALL, int)
    assert not isinstance(Constants.ALL, MessageLevel)
    for level in MessageLevel:
        constant = getattr(Constants, level.name)
        assert constant == level
        assert isinstance(constant, int)
        assert not isinstance(constant, MessageLevel)

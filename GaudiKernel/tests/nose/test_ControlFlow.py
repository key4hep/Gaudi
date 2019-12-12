#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from GaudiConfig.ControlFlow import *
from GaudiKernel.Configurable import Configurable

from test_Configurables import MyAlg


def test_unique_getFullName():
    Configurable.allConfigurables.clear()

    s = MyAlg('a') & MyAlg('b')
    assert s.getFullName() == s.getFullName()

    s = MyAlg('a') >> MyAlg('b')
    assert s.getFullName() == s.getFullName()

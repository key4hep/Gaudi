#####################################################################################
# (c) Copyright 2023 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
import pytest
from GaudiConfig2 import Configurable, useGlobalInstances


@pytest.fixture
def with_global_instances():
    Configurable.instances.clear()
    useGlobalInstances(True)
    yield
    Configurable.instances.clear()
    useGlobalInstances(False)

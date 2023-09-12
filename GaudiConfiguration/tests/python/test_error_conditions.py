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


def test_1():
    from GaudiConfig2.Configurables.TestConf import AlgWithMaps, AlgWithVectors

    alg = AlgWithVectors()
    with pytest.raises(TypeError, match=r"cannot set property VS .*"):
        alg.VS = [3]

    alg = AlgWithMaps()
    with pytest.raises(TypeError, match=r"cannot set property MSS key .*"):
        alg.MSS[3] = "value"
    with pytest.raises(TypeError, match=r"cannot set property MSS value .*"):
        alg.MSS["key"] = 3
    alg.MIV[3] = []
    with pytest.raises(TypeError, match=r"cannot set property MIV value element.*"):
        alg.MIV[3].append(5)

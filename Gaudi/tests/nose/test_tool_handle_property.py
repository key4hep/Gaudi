#####################################################################################
# (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
def test():
    import GaudiKernel.Configurable

    allConfigurables = GaudiKernel.Configurable.Configurable.allConfigurables
    allConfigurables.clear()

    from Configurables import MyGaudiAlgorithm

    assert not allConfigurables

    alg = MyGaudiAlgorithm()
    assert len(allConfigurables) == 1
    assert "MyGaudiAlgorithm.PrivToolHandle" not in allConfigurables

    _ = alg.PrivToolHandle
    assert "MyGaudiAlgorithm.PrivToolHandle" in allConfigurables
    assert "ToolSvc.PrivToolHandle" not in allConfigurables

    print(list(allConfigurables))
    assert len(allConfigurables) == 2

    allConfigurables.clear()

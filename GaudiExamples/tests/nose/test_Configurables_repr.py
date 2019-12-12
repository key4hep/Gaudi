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
def test_repr():
    import Configurables
    assert repr(Configurables.TupleAlg('A')) == "TupleAlg('A')"
    assert repr(Configurables.ToolSvc('B')) == "ToolSvc('B')"
    assert repr(Configurables.TestTool('C')) == "TestTool('ToolSvc.C')"
    assert repr(Configurables.Gaudi__Examples__EvtColAlg()
                ) == "Gaudi__Examples__EvtColAlg('Gaudi::Examples::EvtColAlg')"
    assert repr(Configurables.Gaudi_Test_MySuperAlg()
                ) == "Gaudi_Test_MySuperAlg('Gaudi_Test_MySuperAlg')"

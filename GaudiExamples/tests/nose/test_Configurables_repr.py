def test_repr():
    import Configurables
    assert repr(Configurables.TupleAlg('A')) == "TupleAlg('A')"
    assert repr(Configurables.ToolSvc('B')) == "ToolSvc('B')"
    assert repr(Configurables.TestTool('C')) == "TestTool('ToolSvc.C')"
    assert repr(Configurables.Gaudi__Examples__EvtColAlg()) == "Gaudi__Examples__EvtColAlg('Gaudi::Examples::EvtColAlg')"
    assert repr(Configurables.Gaudi_Test_MySuperAlg()) == "Gaudi_Test_MySuperAlg('Gaudi_Test_MySuperAlg')"

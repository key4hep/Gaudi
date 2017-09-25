from GaudiConfig.ControlFlow import *
from GaudiKernel.Configurable import Configurable

from test_Configurables import MyAlg


def test_unique_getFullName():
    Configurable.allConfigurables.clear()

    s = MyAlg('a') & MyAlg('b')
    assert s.getFullName() == s.getFullName()

    s = MyAlg('a') >> MyAlg('b')
    assert s.getFullName() == s.getFullName()

print('ciao')
from GaudiConfig2.Configurables.TestConf import MyAlg


def confFunction():
    return [MyAlg(AnIntProp=123)]

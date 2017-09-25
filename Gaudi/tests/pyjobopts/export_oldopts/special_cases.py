from Gaudi.Configuration import ConfigurableUser
from GaudiKernel.Configurable import ConfigurableAlgorithm


class MyAlg(ConfigurableAlgorithm):
    __slots__ = {'File': 'filename',
                 'Text': 'text field'}

    def getDlls(self):
        return 'Dummy'

    def getType(self):
        return 'MyAlg'


alg = MyAlg()
alg.File = r'C:\something\vXrY\filename'
alg.Text = '''one line
\tanother "line"'''

"""
Bunch of fake configurables used for testing.
"""

from GaudiKernel.GaudiHandles import *
from GaudiKernel.Proxy.Configurable import *

class APublicTool( ConfigurableAlgTool ):
    __slots__ = {}
    def __init__(self, name = Configurable.DefaultName, **kwargs):
        super(APublicTool, self).__init__(name)
        for n,v in kwargs.items():
            setattr(self, n, v)
    def getDlls( self ):
        return 'None'
    def getType( self ):
        return 'APublicTool'

class APrivateTool( ConfigurableAlgTool ):
    __slots__ = {}
    def __init__(self, name = Configurable.DefaultName, **kwargs):
        super(APrivateTool, self).__init__(name)
        for n,v in kwargs.items():
            setattr(self, n, v)
    def getDlls( self ):
        return 'None'
    def getType( self ):
        return 'APrivateTool'

class MyTestTool( ConfigurableAlgTool ):
    __slots__ = { 
        'PubToolHndl'  : PublicToolHandle('APublicTool'),
        'PrivToolHndl' : PrivateToolHandle('APrivateTool'),
        'PrivEmptyToolHndl' : PrivateToolHandle('')
    }
    def __init__(self, name = Configurable.DefaultName, **kwargs):
        super(MyTestTool, self).__init__(name)
        for n,v in kwargs.items():
            setattr(self, n, v)
    def getDlls( self ):
        return 'None'
    def getType( self ):
        return 'MyTestTool'


###############################################################
# Job options file
###############################################################

from Gaudi.Configuration import *
from Configurables import TimingAuditor, EventLoopMgr

from GaudiKernel.Configurable import SuperAlgorithm

from Configurables import GaudiExamplesCommonConf

GaudiExamplesCommonConf()


class MySuperAlg(SuperAlgorithm):
    '''
    Example implementation of a SuperAlgorithm specialization with behaviour
    depending on a property.

    In this case the boolean option UseHelloWorld defines if the HelloWorld
    instance has to be used or not.
    '''

    def __init__(self, *args, **kwargs):
        # preset the internal flag bypassing __setattr__
        self.__dict__['_hello_flag'] = kwargs.pop('UseHelloWorld', True)
        super(MySuperAlg, self).__init__(*args, **kwargs)

    # define setter and getter callbacks for UseHelloWorld
    @property
    def UseHelloWorld(self):
        return self._hello_flag

    @UseHelloWorld.setter
    def UseHelloWorld(self, value):
        self._hello_flag = value
        # re-generate the graph (which takes into account the flag)
        self.graph = self._initGraph()

    def _initGraph(self):
        '''
        Prepare the graph represented by the SuperAlgorithm.
        '''
        from Configurables import Prescaler, HelloWorld, EventCounter
        p = self._makeAlg(Prescaler, PercentPass=50.)
        h = self._makeAlg(HelloWorld, name='HW')
        c = self._makeAlg(EventCounter, name='Counter')
        # the actual graph depends on the UseHelloWorld flag
        return (p & h & c) if self.UseHelloWorld else (p & c)


s1 = MySuperAlg('s1', OutputLevel=INFO)
s2 = MySuperAlg('s2', OutputLevel=WARNING)
top = s1 >> s2

MySuperAlg('s2', PercentPass=75, OutputLevel=DEBUG, UseHelloWorld=False)


print '# --- Configured Control Flow Expression:'
print '#', top
print '# ---'
EventLoopMgr(PrintControlFlowExpression=True)

# -----------------------------------------------------------------
ApplicationMgr(TopAlg=[top],
               EvtMax=10,      # events to be processed (default is 10)
               EvtSel='NONE',  # do not use any event input
               ExtSvc=['ToolSvc', 'AuditorSvc'],
               AuditAlgorithms=True)

AuditorSvc().Auditors.append(TimingAuditor("TIMER"))

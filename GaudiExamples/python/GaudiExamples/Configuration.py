"""
Module with the ConfigurablesUser for the common configuration in the examples.

@author Marco Clemencic <marco.clemencic@cern.ch>
"""

from GaudiKernel.Configurable import ConfigurableUser, SuperAlgorithm
from Gaudi.Configuration import INFO


class GaudiExamplesCommonConf(ConfigurableUser):
    """Enable common configuration for GaudiExamples tests.
    """
    __slots__ = {"OutputLevel": INFO,
                 "DummyEvents": -1}

    def __apply_configuration__(self):
        from Configurables import (AuditorSvc, ChronoAuditor, ApplicationMgr,
                                   MessageSvc)
        AuditorSvc().Auditors.append(ChronoAuditor())
        appmgr = ApplicationMgr(StatusCodeCheck=True)
        self.propagateProperties(others=MessageSvc())
        if self.getProp('DummyEvents') >= 0:
            appmgr.EvtMax = self.getProp('DummyEvents')
            appmgr.EvtSel = "NONE"


class Gaudi_Test_MySuperAlg(SuperAlgorithm):
    '''
    Example implementation of a SuperAlgorithm specialization.
    '''

    def _initGraph(self):
        '''
        Prepare the graph represented by the SuperAlgorithm.
        '''
        from Configurables import Prescaler, HelloWorld, EventCounter
        p = self._makeAlg(Prescaler, PercentPass=50.)
        h = self._makeAlg(HelloWorld, name='HW')
        c = self._makeAlg(EventCounter, name='Counter')
        return (p & h & c)

"""
Module with the ConfigurablesUser for the common configuration in the examples.
"""
__author__ = "Marco Clemencic <marco.clemencic@cern.ch>"

from GaudiKernel.Configurable import ConfigurableUser
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
        self.propagateProperties(others = MessageSvc())
        if self.getProp('DummyEvents') >= 0:
            appmgr.EvtMax = self.getProp('DummyEvents')
            appmgr.EvtSel = "NONE"

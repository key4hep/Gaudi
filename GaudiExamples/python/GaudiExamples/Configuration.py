#####################################################################################
# (c) Copyright 1998-2022 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
"""
Module with the ConfigurablesUser for the common configuration in the examples.

@author Marco Clemencic <marco.clemencic@cern.ch>
"""

from Gaudi.Configuration import INFO
from GaudiKernel.Configurable import ConfigurableUser, SuperAlgorithm


class GaudiExamplesCommonConf(ConfigurableUser):
    """Enable common configuration for GaudiExamples tests."""

    __slots__ = {"OutputLevel": INFO, "DummyEvents": -1}

    def __apply_configuration__(self):
        from Configurables import ApplicationMgr, AuditorSvc, ChronoAuditor, MessageSvc

        AuditorSvc().Auditors.append(ChronoAuditor())
        appmgr = ApplicationMgr()
        self.propagateProperties(others=MessageSvc())
        if self.getProp("DummyEvents") >= 0:
            appmgr.EvtMax = self.getProp("DummyEvents")
            appmgr.EvtSel = "NONE"


class Gaudi_Test_MySuperAlg(SuperAlgorithm):
    """
    Example implementation of a SuperAlgorithm specialization.
    """

    def _initGraph(self):
        """
        Prepare the graph represented by the SuperAlgorithm.
        """
        from Configurables import Gaudi__Examples__EventCounter as EventCounter
        from Configurables import Gaudi__Examples__Prescaler as Prescaler
        from Configurables import HelloWorld

        p = self._makeAlg(Prescaler, name="Prescaler", PercentPass=50.0)
        h = self._makeAlg(HelloWorld, name="HW")
        c = self._makeAlg(EventCounter, name="Counter")
        return p & h & c

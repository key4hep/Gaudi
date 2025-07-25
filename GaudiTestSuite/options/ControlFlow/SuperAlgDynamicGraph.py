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

from Configurables import AlgTimingAuditor, EventLoopMgr, GaudiTestSuiteCommonConf
from Gaudi.Configuration import *
from GaudiKernel.Configurable import SuperAlgorithm

###############################################################
# Job options file
###############################################################


GaudiTestSuiteCommonConf()


class MySuperAlg(SuperAlgorithm):
    """
    Example implementation of a SuperAlgorithm specialization with behaviour
    depending on a property.

    In this case the boolean option UseHelloWorld defines if the HelloWorld
    instance has to be used or not.
    """

    def __init__(self, *args, **kwargs):
        # preset the internal flag bypassing __setattr__
        self.__dict__["_hello_flag"] = kwargs.pop("UseHelloWorld", True)
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
        """
        Prepare the graph represented by the SuperAlgorithm.
        """
        from Configurables import Gaudi__TestSuite__EventCounter as EventCounter
        from Configurables import Gaudi__TestSuite__Prescaler as Prescaler
        from Configurables import HelloWorld

        p = self._makeAlg(Prescaler, name="Prescaler", PercentPass=50.0)
        h = self._makeAlg(HelloWorld, name="HW")
        c = self._makeAlg(EventCounter, name="Counter")
        # the actual graph depends on the UseHelloWorld flag
        return (p & h & c) if self.UseHelloWorld else (p & c)


s1 = MySuperAlg("s1", OutputLevel=INFO)
s2 = MySuperAlg("s2", OutputLevel=WARNING)
top = s1 >> s2

MySuperAlg("s2", PercentPass=75, OutputLevel=DEBUG, UseHelloWorld=False)

print("# --- Configured Control Flow Expression:")
print("#", top)
print("# ---")
EventLoopMgr(PrintControlFlowExpression=True)

# -----------------------------------------------------------------
ApplicationMgr(
    TopAlg=[top],
    EvtMax=10,  # events to be processed (default is 10)
    EvtSel="NONE",  # do not use any event input
    ExtSvc=["ToolSvc", "AuditorSvc", "Gaudi::Monitoring::MessageSvcSink"],
    AuditAlgorithms=True,
)

AuditorSvc().Auditors.append(AlgTimingAuditor("TIMER"))

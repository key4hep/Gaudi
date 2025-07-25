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
Example of usage of ConfigurableUser classes (for high level configuration).
"""

from Gaudi.Configuration import *


class ExampleIO(ConfigurableUser):
    __slots__ = {"EvtMax": 1, "Read": "NONE", "Write": "NONE"}
    __used_configurables__ = [ApplicationMgr]

    def __apply_configuration__(self):
        # Propagate the properties to all used configurables
        # In this case the only effect is to set the ApplicationMgr EvtMax.
        self.propagateProperties()

        read = self.getProp("Read")
        if read == "NONE":
            ApplicationMgr(EvtSel="NONE")
        else:
            log.warning(
                "%s: Cannot handle value %r of property Read", self.name(), read
            )

        write = self.getProp("Write")
        if read == "NONE":
            ApplicationMgr(HistogramPersistency="NONE")
        else:
            log.warning(
                "%s: Cannot handle value %r of property Write", self.name(), write
            )


class ExampleCommon(ConfigurableUser):
    __slots__ = {"OutputLevel": INFO}
    __used_configurables__ = []

    def __apply_configuration__(self):
        MessageSvc().OutputLevel = self.getProp("OutputLevel")


class ExampleApplication(ConfigurableUser):
    __slots__ = {
        "FullDebug": False,
        "TopAlg": [],
    }
    __used_configurables__ = [ExampleCommon, ApplicationMgr]

    def __apply_configuration__(self):
        if self.getProp("FullDebug"):
            ExampleCommon(OutputLevel=VERBOSE)
        else:
            ExampleCommon(OutputLevel=INFO)
        # Propagate the properties to all used configurables
        # In this case the only effect is to set the ApplicationMgr TopAlg.
        self.propagateProperties()


def PostConfAction():
    """
    Action printing the result of the configuration of the ApplicationMgr.
    """
    print("==== Configuration completed ====")
    print(ApplicationMgr())


appendPostConfigAction(PostConfAction)

ExampleApplication(TopAlg=["GaudiTestSuite::TimingAlg/Timing"], FullDebug=True)
ExampleIO(EvtMax=10)

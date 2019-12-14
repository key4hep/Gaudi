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
from __future__ import print_function
from Gaudi.Configuration import *


class DelayedInstance(ConfigurableUser):
    __slots__ = {"Property": 0, "Applied": False}

    def __apply_configuration__(self):
        print("Applying", self.getName())
        print(self)
        self.Applied = True


class Application(ConfigurableUser):
    __slots__ = {"Property": 10, "Applied": False}
    __used_configurables__ = []

    def __apply_configuration__(self):
        print("Applying", self.getName())
        print(self)
        self.Applied = True
        # This is instantiated late
        DelayedInstance()


# Instantiate the application configurable
Application()

# apply all ConfigurableUser instances
from GaudiKernel.Configurable import applyConfigurableUsers
applyConfigurableUsers()

assert Application().Applied
assert DelayedInstance().Applied

print("Done.")

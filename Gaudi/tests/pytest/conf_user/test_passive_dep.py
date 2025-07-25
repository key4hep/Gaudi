#####################################################################################
# (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
from Gaudi.Configuration import ConfigurableUser

applied_order = []


class CommonConf(ConfigurableUser):
    __slots__ = {"Property1": 10, "Property2": 10}

    def __apply_configuration__(self):
        global applied_order
        applied_order.append(self.name())


class Application(ConfigurableUser):
    __slots__ = {"Property1": 10}
    __queried_configurables__ = [CommonConf]

    def __apply_configuration__(self):
        global applied_order
        applied_order.append(self.name())


def test():
    # Instantiate the application configurable
    Application()
    CommonConf()

    # apply all ConfigurableUser instances
    from GaudiKernel.Configurable import applyConfigurableUsers

    applyConfigurableUsers()
    print("Done.")

    expected_order = ["CommonConf", "Application"]

    assert applied_order == expected_order

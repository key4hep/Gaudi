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
from pprint import pprint

from Gaudi.Configuration import (
    Configurable,
    ConfigurableAlgTool,
    ConfigurableUser,
    configurationDict,
)
from GaudiKernel.Configurable import applyConfigurableUsers

# use cases:
#     parent, used
#   A set   , set
#   B set   , unset
#   C unset , set
#   D unset , unset

PS = "Parent Set"
PU = "Parent Unset"
CS = "Child Set"
CU = "Child Unset"


class PlainConfigurable(ConfigurableAlgTool):
    __slots__ = {
        "IntA": CU,
        "IntB": CU,
        "IntC": CU,
        "IntD": CU,
        "LstA": [CU],
        "LstB": [CU],
        "LstC": [CU],
        "LstD": [CU],
    }

    def __init__(self, name=Configurable.DefaultName, **kwargs):
        super(PlainConfigurable, self).__init__(name)
        for n, v in kwargs.items():
            setattr(self, n, v)

    def getDlls(self):
        return "None"

    def getType(self):
        return "PlainConfigurable"


class Master(ConfigurableUser):
    __slots__ = {
        "IntA": PU,
        "IntB": PU,
        "IntC": PU,
        "IntD": PU,
        "LstA": [PU],
        "LstB": [PU],
        "LstC": [PU],
        "LstD": [PU],
    }

    def __apply_configuration__(self):
        self.propagateProperties(others=[PlainConfigurable()])

    def getGaudiType(self):
        return "Test"


class SubModule(ConfigurableUser):
    __slots__ = {
        "IntA": CU,
        "IntB": CU,
        "IntC": CU,
        "IntD": CU,
        "LstA": [CU],
        "LstB": [CU],
        "LstC": [CU],
        "LstD": [CU],
    }

    def getGaudiType(self):
        return "Test"


class SuperModule(ConfigurableUser):
    __slots__ = {
        "IntA": PU,
        "IntB": PU,
        "IntC": PU,
        "IntD": PU,
        "LstA": [PU],
        "LstB": [PU],
        "LstC": [PU],
        "LstD": [PU],
    }
    __used_configurables__ = [SubModule]

    def __apply_configuration__(self):
        self.propagateProperties()

    def getGaudiType(self):
        return "Test"


def test():
    pc = PlainConfigurable(IntA=CS, IntC=CS, LstA=[CS], LstC=[CS])
    _m = Master(IntA=PS, IntB=PS, LstA=[PS], LstB=[PS])

    sub = SubModule(IntA=CS, IntC=CS, LstA=[CS], LstC=[CS])
    _sup = SuperModule(IntA=PS, IntB=PS, LstA=[PS], LstB=[PS])

    # apply all ConfigurableUser instances
    applyConfigurableUsers()

    pprint(configurationDict())

    for conf, prop, exp in [
        (pc, "IntA", PS),
        (pc, "IntB", PS),
        (pc, "IntC", CS),
        (pc, "IntD", PU),
        (sub, "IntA", PS),
        (sub, "IntB", PS),
        (sub, "IntC", CS),
        (sub, "IntD", PU),
        (pc, "LstA", [PS]),
        (pc, "LstB", [PS]),
        (pc, "LstC", [CS]),
        (pc, "LstD", [PU]),
        (sub, "LstA", [PS]),
        (sub, "LstB", [PS]),
        (sub, "LstC", [CS]),
        (sub, "LstD", [PU]),
    ]:
        v = conf.getProp(prop)
        assert (
            v == exp
        ), f"{conf.name()}.{prop} is {v!r} (expected {exp!r}), {'set' if hasattr(conf, prop) else 'unset'}"

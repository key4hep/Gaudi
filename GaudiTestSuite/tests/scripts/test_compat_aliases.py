#!/usr/bin/env python3
#####################################################################################
# (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
"""
Created on Feb 3, 2014

@author: Marco Clemencic
"""

import re


def old_name(name):
    """
    Implement the old-style class name translation.
    """
    return re.sub(
        "[<>:,().]", "_", name.replace(" ", "").replace("&", "r").replace("*", "p")
    )


def test_aliases():
    import GaudiPluginService.cpluginsvc

    factories = GaudiPluginService.cpluginsvc.factories()
    # components to check
    components = [
        "PluginServiceTest::MyAlg",
        "PluginServiceTest::MyTemplatedAlg<int&, double*>",
    ]
    class_map = dict(zip(components, components))
    # add old style names
    class_map.update([(old_name(name), name) for name in components])

    for component in sorted(class_map):
        assert component in factories, "missing component %s" % component
        factories[component].load()
        assert factories[component].classname == class_map[component], (
            'expected class "%s", found "%s"'
            % (
                class_map[component],
                factories[component].classname,
            )
        )


if __name__ == "__main__":
    test_aliases()

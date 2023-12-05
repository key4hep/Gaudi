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
"""
Hand-written confDb file for tests.
"""


def _fillCfgDb():
    from GaudiKernel.Proxy.ConfigurableDb import CfgDb

    # get a handle on the repository of Configurables
    cfgDb = CfgDb()
    cfgDb.add(
        configurable="MyTestTool", package="GaudiTests", module="TestConf", lib="None"
    )
    cfgDb.add(
        configurable="APublicTool", package="GaudiTests", module="TestConf", lib="None"
    )
    cfgDb.add(
        configurable="APrivateTool", package="GaudiTests", module="TestConf", lib="None"
    )

    return  # _fillCfgDb


# fill cfgDb at module import...
try:
    _fillCfgDb()
    # house cleaning...
    del _fillCfgDb
except Exception as err:
    print("Py:ConfigurableDb   ERROR Problem with [%s] content!" % __name__)
    print("Py:ConfigurableDb   ERROR", err)
    print("Py:ConfigurableDb   ERROR   ==> culprit is package [Test] !")

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
Module used to select if to use AthenCommon or GaudiKernel to retrieve the
implementation of Configurables.

@author Marco Clemencic
"""

try:
    # Try to import AthenaCommon
    from AthenaCommon import Configurable, ConfigurableDb

    def _getNeededConfigurables():
        """
        In Athena, the list of configurables to be used in GaudiPython.AppMgr
        constructor is empty, because the configuration is done in a different
        place.
        """
        return []

    Configurable.getNeededConfigurables = _getNeededConfigurables
except ImportError:
    # Otherwise use GaudiKernel
    from GaudiKernel import Configurable, ConfigurableDb

# Hack to expose Configurable and ConfigurableDb as the modules
# GaudiKernel.Proxy.Configurable and GaudiKernel.Proxy.ConfigurableDb
import sys

sys.modules["GaudiKernel.Proxy.Configurable"] = Configurable
sys.modules["GaudiKernel.Proxy.ConfigurableDb"] = ConfigurableDb
import GaudiKernel.Proxy.Configurable
import GaudiKernel.Proxy.ConfigurableDb  # noqa: F401

del sys

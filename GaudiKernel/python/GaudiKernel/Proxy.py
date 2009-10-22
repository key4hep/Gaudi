"""
Module used to select if to use AthenCommon or GaudiKernel to retrieve the
implementation of Configurables.

@author Marco Clemencic
"""

try:
    # Try to import AthenaCommon
    from AthenaCommon import Configurable
    from AthenaCommon import ConfigurableDb
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
    from GaudiKernel import Configurable
    from GaudiKernel import ConfigurableDb

# Hack to expose Configurable and ConfigurableDb as the modules
# GaudiKernel.Proxy.Configurable and GaudiKernel.Proxy.ConfigurableDb
import sys
sys.modules["GaudiKernel.Proxy.Configurable"] = Configurable
sys.modules["GaudiKernel.Proxy.ConfigurableDb"] = ConfigurableDb
import GaudiKernel.Proxy.Configurable
import GaudiKernel.Proxy.ConfigurableDb
del sys

"""
Hand-written confDb file for tests.
"""
from __future__ import print_function


def _fillCfgDb():
    from GaudiKernel.Proxy.ConfigurableDb import CfgDb

    # get a handle on the repository of Configurables
    cfgDb = CfgDb()
    cfgDb.add(
        configurable='MyTestTool',
        package='GaudiTests',
        module='TestConf',
        lib='None')
    cfgDb.add(
        configurable='APublicTool',
        package='GaudiTests',
        module='TestConf',
        lib='None')
    cfgDb.add(
        configurable='APrivateTool',
        package='GaudiTests',
        module='TestConf',
        lib='None')

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

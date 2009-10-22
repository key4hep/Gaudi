# File: Gaudi/CommonGaudiConfigurables.py
# Author: Pere Mato (per.mato@cern.ch)

"""
  This module would scan all known Gaudi configurable modules for
  'Configurable' classes and fill __all__ such that it can be imported
  by any module requiring it.
"""

from GaudiKernel.Configurable import Configurable
from GaudiKernel.ConfigurableMeta import ConfigurableMeta
__all__ = []

packages = ['GaudiSvc', 'GaudiAlg', 'GaudiAud', 'GaudiPoolDb', 'RootHistCnv',
            'GaudiUtils' ]

#--Loop open all listed packages and populate __all__ with the names and
#  the local scope with the Configurable classes
for package in packages :
    try:
        mod = __import__( '%s.%sConf'%(package,package), globals(), locals(), ['%sConf'%package] )
        for nam in dir(mod) :
            cls = getattr(mod, nam)
            if type(cls) is ConfigurableMeta and issubclass(cls, Configurable) :
                globals()[nam]  =  cls
                __all__.append(nam)
    except ImportError:
        # ignore the configurables from missing packages.
        pass

#--Fix some of the name idiosyncrasies in Gaudi
def addConfigurableAs(ori, new) :
    gbl = globals()
    if ori in gbl: # do the aliasing only if the original is available
        gbl[new] = gbl[ori]
        __all__.append(new)
        gbl[new].DefaultedName = new

addConfigurableAs('EvtDataSvc','EventDataSvc')
addConfigurableAs('DetDataSvc','DetectorDataSvc')
addConfigurableAs('HistogramSvc','HistogramDataSvc')
addConfigurableAs('HbookCnv__PersSvc','HbookHistSvc')
addConfigurableAs('RootHistCnv__PersSvc','RootHistSvc')
addConfigurableAs('EvtPersistencySvc','EventPersistencySvc')
addConfigurableAs('DetPersistencySvc','DetectorPersistencySvc')
addConfigurableAs('HistogramPersistencySvc','HistogramPersistencySvc')

addConfigurableAs('Gaudi__MultiFileCatalog','FileCatalog')
addConfigurableAs('Gaudi__IODataManager','IODataManager')

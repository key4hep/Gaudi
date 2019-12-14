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
# File: Gaudi/CommonGaudiConfigurables.py
# Author: Pere Mato (pere.mato@cern.ch)
"""
  This module would scan all known Gaudi configurable modules for
  'Configurable' classes and fill __all__ such that it can be imported
  by any module requiring it.
"""

from GaudiKernel.Configurable import Configurable
from GaudiKernel.ConfigurableMeta import ConfigurableMeta
__all__ = []

packages = [
    'GaudiCoreSvc', 'GaudiCommonSvc', 'GaudiSvc', 'GaudiAlg', 'GaudiAud',
    'GaudiPoolDb', 'RootHistCnv', 'GaudiUtils', 'RootCnv'
]

# --Loop open all listed packages and populate __all__ with the names and
#  the local scope with the Configurable classes
for package in packages:
    try:
        mod = __import__('%s.%sConf' % (package, package), globals(), locals(),
                         ['%sConf' % package])
        for nam in dir(mod):
            cls = getattr(mod, nam)
            if type(cls) is ConfigurableMeta and issubclass(cls, Configurable):
                globals()[nam] = cls
                __all__.append(nam)
    except ImportError:
        # ignore the configurables from missing packages.
        pass

# --Fix some of the name idiosyncrasies in Gaudi
aliases = {
    'EventDataSvc': 'EvtDataSvc',
    'DetectorDataSvc': 'DetDataSvc',
    'HistogramDataSvc': 'HistogramSvc',
    'HbookHistSvc': 'HbookCnv__PersSvc',
    'RootHistSvc': 'RootHistCnv__PersSvc',
    'EventPersistencySvc': 'EvtPersistencySvc',
    'DetectorPersistencySvc': 'DetPersistencySvc',
    'HistogramPersistencySvc': 'HistogramPersistencySvc',
    'FileRecordPersistencySvc': 'PersistencySvc',
    'FileCatalog': 'Gaudi__MultiFileCatalog',
    'IODataManager': 'Gaudi__IODataManager',
    'RootCnvSvc': 'Gaudi__RootCnvSvc',
    'RootEvtSelector': 'Gaudi__RootEvtSelector',
}

_gbl = globals()  # optimization
# This would be nicer with dict comprehension (http://www.python.org/dev/peps/pep-0274)
# but it is available only in Python 2.7
aliases = dict([(new, _gbl[old]) for new, old in aliases.items() if old in _gbl
                ])  # do the aliasing only if the original is available
# change the default name
for new in aliases:
    aliases[new].DefaultedName = new
# update globals and __all__
_gbl.update(aliases)
__all__.extend(aliases)
# remove temporaries
del _gbl
# The `new` var is only scoped in its `for` loop in Python 3, so we only need
# to 'worry' about cleanup in Python 2
try:
    del new
except NameError:
    pass

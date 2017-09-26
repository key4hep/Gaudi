#!/usr/bin/env python
# =============================================================================
"""
*******************************************************************************
*                                                                             *
* Simple example which illustrate the 'boost::array'-job-properties and their *
*   C++/Python intercommunications                                            *
*                                                                             *
*******************************************************************************
"""
# =============================================================================
__author__ = 'Vanya BELYAEV Ivan.Belyaev@nikhef.nl'
# =============================================================================
# @file
#  Simple which illustrates
#  the 'array-like' job-properties and their C++/Python intercommunication
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date 2007-02-13
# =============================================================================
from Gaudi.Configuration import *

from Configurables import ApplicationMgr

from Configurables import Gaudi__Examples__BoostArrayProperties as BAP


bap = BAP(
    'BoostArrayProps', Strings=('a', 'bb', 'ccc', 'dddd'), Doubles=(1, 2, 3, 4, 5)
)

ApplicationMgr(
    EvtSel="NONE", TopAlg=[bap], EvtMax=10
)

# =============================================================================
# The actual job excution
# =============================================================================
if '__main__' == __name__:

    print __doc__, __author__

    from GaudiPython.Bindings import AppMgr

    gaudi = AppMgr()

    gaudi.run(5)

    bap = gaudi.algorithm('BoostArrayProps')

    bap.PropertiesPrint = True

    from GaudiKernel import ROOT6WorkAroundEnabled
    import warnings
    with warnings.catch_warnings():
        if ROOT6WorkAroundEnabled('ROOT-7142'):
            warnings.simplefilter("ignore")
        bap.Doubles = [-1, -2, -3, -4, -5]
        bap.Strings = ['a1', 'a2', 'a3', 'a4']

    bap.PropertiesPrint = True


# =============================================================================
# The END
# =============================================================================

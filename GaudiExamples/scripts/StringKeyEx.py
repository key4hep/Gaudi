#!/usr/bin/env python
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
# =============================================================================
"""
*******************************************************************************
*                                                                             *
* Simple example which illustrates the usage of class StringKey               *
*                                                                             *
*******************************************************************************
"""
from __future__ import print_function
# =============================================================================
__author__ = 'Vanya BELYAEV Ivan.Belyaev@nikhef.nl'
# =============================================================================
# @file
#  Simple example which illustrates the usage of class StringKey
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date 2009-10-07
# =============================================================================
from Gaudi.Configuration import *

from Configurables import ApplicationMgr

from Configurables import Gaudi__Examples__StringKeyEx as SKE

ske = SKE('StringKeys', Key='key', Keys=['a', 'b', 'key', 'c', 'd'])

ApplicationMgr(EvtSel="NONE", TopAlg=[ske], EvtMax=10)

# =============================================================================
# The actual job excution
# =============================================================================
if '__main__' == __name__:

    print(__doc__)
    print(__author__)

    from GaudiPython.Bindings import AppMgr

    gaudi = AppMgr()

    gaudi.run(1)

    ske = gaudi.algorithm('StringKeys')

    ske.PropertiesPrint = True

    from GaudiPython.Bindings import gbl as cpp

    SK = cpp.Gaudi.StringKey

    key = SK('new Key')

    print('set new key: ', key)

    ske.Key = key

    keys = [key, 'rrr', SK('s')]

    print('set new keys: ', keys)

    ske.Keys = keys

    ske.PropertiesPrint = True

    gaudi.run(1)

# =============================================================================
# The END
# =============================================================================

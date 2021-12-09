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
* Simple example which illustrate the 'array'-like job-properties and their   *
*   C++/Python intercommunications                                            *
*                                                                             *
*******************************************************************************
"""
from __future__ import print_function

# =============================================================================
__author__ = "Vanya BELYAEV Ivan.Belyaev@nikhef.nl"
from Configurables import ApplicationMgr
from Configurables import Gaudi__Examples__ArrayProperties as AP

# =============================================================================
# @file
#  Simple which illustrates
#  the 'array-like' job-properties and their C++/Python intercommunication
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date 2007-02-13
# =============================================================================
from Gaudi.Configuration import *

ap = AP("ArrayProps", Strings=("a", "bb", "ccc", "dddd"), Doubles=(1, 2, 3, 4, 5))

ApplicationMgr(EvtSel="NONE", TopAlg=[ap], EvtMax=10)

# =============================================================================
# The actual job excution
# =============================================================================
if "__main__" == __name__:

    print(__doc__, __author__)

    # make sure cling can generate all required methods in Gaudi::Property
    # Workaround for ROOT-10769
    import warnings

    with warnings.catch_warnings():
        warnings.simplefilter("ignore")
        import cppyy
    cppyy.gbl.gInterpreter.Declare("#define NO_C_ARRAY_AS_PROPERTY_WARNING")
    cppyy.gbl.gInterpreter.Declare('#include "GaudiKernel/CArrayAsProperty.h"')

    from GaudiPython.Bindings import AppMgr

    gaudi = AppMgr()

    gaudi.run(5)

    ap = gaudi.algorithm("ArrayProps")

    ap.PropertiesPrint = True

    ap.Strings = ["qu-qu", "qu-qu", "qu-qu", "qu-qu"]
    ap.Doubles = [0, -1, 2, -3, 4]

    ap.PropertiesPrint = True

# =============================================================================
# The END
# =============================================================================

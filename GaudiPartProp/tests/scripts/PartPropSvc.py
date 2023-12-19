#!/usr/bin/env python
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
# =============================================================================
## @file
#  The configuration file to run QMTest for the package Kernel/PartProp
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date   2008-12-02
# =============================================================================
"""
The configuration file to run QMTest for the package Kernel/PartProp
"""

__author__ = "Vanya BELYAEV Ivan.Belyaev@nikhef.nl"
__version__ = ""

from PartProp.PartPropSvc import test

test()

#!/usr/bin/env python3
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
## @file GaudiPartProp/__init__.py
#  Helper file/module for GaudiPartProp package
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  date 2008-12-01
# =============================================================================
"""
Helper file/module for GaudiPartProp package
"""

# =============================================================================
__author__ = "Vanya BELYAEV Ivan.Belyaev@nikhef.nl"
__version__ = ""
# =============================================================================
# The following is misunderstood by flake8 - the import is needed as it
# defines this for module scripts
from ._default_table import DEFAULT_PARTICLE_PROPERTY_FILE  # noqa: F401

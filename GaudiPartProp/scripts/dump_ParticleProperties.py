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
# =============================================================================
## @file dump_ParticleProperties
#
#  Trivial script to dump the particle properties
#
#  @date   2010-10-22
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#
#  Last modification $Date: 2010-09-10 14:04:49 $
#                 by $Author: ibelyaev $
# =============================================================================
import sys

from PartProp.DumpParticleProperties import dump

dump([a for a in sys.argv[1:] if "--" != a])

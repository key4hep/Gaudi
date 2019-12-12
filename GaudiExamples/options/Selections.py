#!/usr/bin/env gaudirun.py
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
from Gaudi.Configuration import ApplicationMgr

from Configurables import Gaudi__Examples__SelCreate as SelCreate
from Configurables import Gaudi__Examples__SelFilter as SelFilter

ApplicationMgr(
    EvtMax=20,
    EvtSel='NONE',
    TopAlg=[
        SelCreate('Create1'),
        SelFilter('Filter1', Input='Create1'),
        SelFilter('Filter2', Input='Filter1'),
        SelFilter('Filter3', Input='Filter2'),
        SelFilter('Filter4', Input='Filter3'),
        SelFilter('Filter5', Input='Filter4')
    ])

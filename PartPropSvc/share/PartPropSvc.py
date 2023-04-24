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
# --------------------------------------------------------------
# PartPropSvc Service options
# --------------------------------------------------------------
from PartPropSvc.PartPropSvcConf import PartPropSvc

ServiceMgr += PartPropSvc()  # noqa: F821 (ATLAS/athena-specific global)
PartPropSvc.InputFile = "PDGTABLE.MeV=PDG"

# --------------------------------------------------------------

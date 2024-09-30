#####################################################################################
# (c) Copyright 2024 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# --- Gaudi related settings ---
gaudi_add_header_only_library(GaudiRustBindings
    LINK
        Gaudi::GaudiKernel
)

# Declare tests
# gaudi_add_pytest(tests/pytest)

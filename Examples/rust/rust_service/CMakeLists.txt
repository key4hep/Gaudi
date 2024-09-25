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
gaudi_add_header_only_library(example_rust_service
    LINK
        Gaudi::GaudiKernel
)

# Declare a Gaudi plugin library (module)
gaudi_add_module(example_rust_service_module
    SOURCES
        src/CppKeyValueStore.cpp
        src/KVSTestAlg.cpp
    LINK
        example_rust_service
        fmt::fmt
        Gaudi::GaudiKernel
)

# Declare tests
gaudi_add_pytest(tests/pytest)

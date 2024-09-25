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
gaudi_add_header_only_library(example_rust_service_headers
    LINK
        Gaudi::GaudiKernel
)

# Declare a Gaudi plugin library (module)
gaudi_add_module(example_rust_service_module
    SOURCES
        src/CppKeyValueStore.cpp
        src/KVSTestAlg.cpp
        src/RustServiceBridge.cpp
    LINK
        example_rust_service_headers
        example_rust_service_bridge
        fmt::fmt
        Gaudi::GaudiKernel
)

# Declare tests
gaudi_add_pytest(tests/pytest)


# --- Rust related settings ---

# Instruct CMake to build the Rust library
corrosion_import_crate(
    MANIFEST_PATH example_rust_service/Cargo.toml
    LOCKED
)
# Instruct CMake to build the bridge code and build the library that
# we use from C++ to invoke Rust code
corrosion_add_cxxbridge(
    example_rust_service_bridge
    CRATE example_rust_service
    FILES lib.rs
)
target_link_libraries(example_rust_service_bridge PRIVATE Gaudi::GaudiKernel)
set_target_properties(example_rust_service_bridge PROPERTIES
                        POSITION_INDEPENDENT_CODE YES
                        )

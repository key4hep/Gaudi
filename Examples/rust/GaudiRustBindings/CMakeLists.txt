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
gaudi_add_library(GaudiRustBindings
    SOURCES
        src/AlgWrapper.cpp
    LINK
        PUBLIC
            Gaudi::GaudiKernel
        PRIVATE
            gaudi_rust_bindings_bridge
)

# Declare tests
# gaudi_add_pytest(tests/pytest)

# --- Rust related settings ---

# Instruct CMake to build the Rust library
corrosion_import_crate(
    MANIFEST_PATH Cargo.toml
    LOCKED
)
# Instruct CMake to build the bridge code and build the library that
# we use from C++ to invoke Rust code
corrosion_add_cxxbridge(
    gaudi_rust_bindings_bridge
    CRATE gaudi_rust_bindings
    FILES lib.rs
)
target_link_libraries(gaudi_rust_bindings_bridge
    PUBLIC
        Gaudi::GaudiKernel
)
target_include_directories(gaudi_rust_bindings_bridge PRIVATE include  PUBLIC src)

# We are building a static library that we want to link into a
# shared library so we need generate position independent code (-fPIC)
set_target_properties(gaudi_rust_bindings_bridge
    PROPERTIES
        POSITION_INDEPENDENT_CODE YES
)

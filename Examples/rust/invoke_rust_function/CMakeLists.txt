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
# Declare a Gaudi plugin library (module)
gaudi_add_module(example_invoke_rust_function
    SOURCES
        src/RustyAlg.cpp
    LINK
        Gaudi::GaudiKernel
        example_rust_lib_bridge # Rust bridge library (defined later)
)

# Declare Python modules and tests
gaudi_install(PYTHON)
gaudi_add_pytest(tests/pytest)


# --- Rust related settings ---

# Instruct CMake to build the Rust library
corrosion_import_crate(
    MANIFEST_PATH example_rust_lib/Cargo.toml
    LOCKED
)
# Instruct CMake to build the bridge code and build the library that
# we use from C++ to invoke Rust code
corrosion_add_cxxbridge(
    example_rust_lib_bridge
    CRATE example_rust_lib
    FILES lib.rs
)


if(BUILD_TESTING)
    # extra testing not supported by Corrosion
    # FIXME it would be nice to pre-build for test and clippy during build and not during tests
    add_test(NAME invoke_rust_function.example_rust_lib.test
        COMMAND Rust::Cargo test --target-dir ${CMAKE_CURRENT_BINARY_DIR}/example_rust_lib --locked
            --manifest-path ${CMAKE_CURRENT_SOURCE_DIR}/example_rust_lib/Cargo.toml
    )
    add_test(NAME invoke_rust_function.example_rust_lib.clippy
        COMMAND Rust::Cargo clippy --target-dir ${CMAKE_CURRENT_BINARY_DIR}/example_rust_lib --locked
            --manifest-path ${CMAKE_CURRENT_SOURCE_DIR}/example_rust_lib/Cargo.toml
    )
endif()

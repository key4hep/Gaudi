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

# Declare a Gaudi plugin library (module)
gaudi_add_module(example_rust_algorithm_module
    SOURCES
        src/RustAlgFactories.cpp
        src/MyCppCountingAlg.cpp
        src/helpers.cpp
    LINK
        Gaudi::GaudiKernel
        Gaudi::GaudiRustBindings
        example_rust_algorithm_bridge
)

# Declare tests
gaudi_add_pytest(tests/pytest)


# --- Rust related settings ---

# Instruct CMake to build the Rust library
corrosion_import_crate(
    MANIFEST_PATH example_rust_algorithm/Cargo.toml
    LOCKED
)

# We are building a static library that we want to link into a
# shared library so we need generate position independent code (-fPIC)
# set_target_properties(example_rust_algorithm_bridge
#     PROPERTIES
#         POSITION_INDEPENDENT_CODE YES
# )
# Instruct CMake to build the bridge code and build the library that
# we use from C++ to invoke Rust code
corrosion_add_cxxbridge(
    example_rust_algorithm_bridge
    CRATE example_rust_algorithm
    FILES lib.rs
)
target_link_libraries(example_rust_algorithm_bridge
    PUBLIC
        Gaudi::GaudiKernel
)
target_include_directories(example_rust_algorithm_bridge PRIVATE src)

# We are building a static library that we want to link into a
# shared library so we need generate position independent code (-fPIC)
set_target_properties(example_rust_algorithm_bridge
    PROPERTIES
        POSITION_INDEPENDENT_CODE YES
)

# --- Benchmarking ---
gaudi_add_executable(rust_algorithm_benchmark
    SOURCES
        src/rust_algorithm_benchmark.cpp
    LINK
        Gaudi::GaudiKernel
)

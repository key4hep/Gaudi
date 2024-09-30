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
if(NOT GAUDI_RUST_EXAMPLES)
    return()
endif()

# FIXME These are settings needed in LHCb builds to avoid warnings
set(Rust_TOOLCHAIN 1.80.0-x86_64-unknown-linux-gnu)
set(Rust_CARGO_TARGET x86_64-unknown-linux-gnu)
set(Rust_RESOLVE_RUSTUP_TOOLCHAINS NO)
set(CMAKE_CONFIGURATION_TYPES)
#


# Add Corrosion CMake modules to easily build and link Rust crates from CMake
include(FetchContent)
FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
    GIT_TAG v0.5 # Optionally specify a commit hash, version tag or branch here
)
# Set any global configuration variables such as `Rust_TOOLCHAIN` before this line!
FetchContent_MakeAvailable(Corrosion)

add_subdirectory(invoke_rust_function)
add_subdirectory(rust_service)
add_subdirectory(rust_algorithms)

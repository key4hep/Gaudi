#[========================================================================[.rst:
dummy LHCbFullStack project
---------------------------

This dummy project aims at showing it is possible to build the
LHCb stack at once.

#]========================================================================]
cmake_minimum_required(VERSION 3.15)

project(dummyLHCbFullStack
        LANGUAGES CXX) # not to check the C compiler because we only use C++

# Add the projects of the stack (from bottom to top)
add_subdirectory(${SRC_DIR} ${BIN_DIR}/dummyLHCbFullStack/Gaudi)
add_subdirectory(${SRC_DIR}/cmake/tests/dummyGaudiDownstreamProject ${BIN_DIR}/dummyLHCbFullStack/dummyGaudiDownstreamProject)

message(STATUS "dummyLHCbFullStack configured successfully")

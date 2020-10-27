#####################################################################################
# (c) Copyright 2020 CERN for the benefit of the LHCb and ATLAS collaborations      #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
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

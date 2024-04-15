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
cmake_minimum_required(VERSION 3.19)

project(mylee_sim
    VERSION 0.1.0
    DESCRIPTION "My Little Example Experiment - simulation application"
    LANGUAGES CXX
)

# Usually we get Gaudi as a dependency via `find_package(Gaudi)`,
# but this example code might be built as part of the Gaudi project,
# so we check if the targets we need are already defined before trying
# a lookup.
if(NOT TARGET Gaudi::GaudiKernel)
    find_package(Gaudi REQUIRED)
endif()
if(NOT TARGET mylee_core::mylee_em)
    find_package(mylee REQUIRED)
endif()
find_package(fmt REQUIRED)

include(CTest)

add_subdirectory(Gen)

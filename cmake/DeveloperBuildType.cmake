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
Build type: Developer
---------------------

When included, this file adds a build type called "Developer" to the
CMake configuration.

This build type is a debug and warnings build type for C++.

#]========================================================================]

include_guard(GLOBAL) # can only be included once per configuration

set(_config Developer)
string(TOUPPER ${_config} _CONFIG)

# Set compile flags
set(_flags "-g -Wall -Wextra -pedantic -Wnon-virtual-dtor -Werror=return-type -Wwrite-strings -Wpointer-arith -Woverloaded-virtual")
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
  (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 11) )
    set(_flags "${_flags} -Wsuggest-override")
endif()
# WARNING: The default value of a cached variable cannot be modified after it has been set
set(CMAKE_CXX_FLAGS_${_CONFIG} "${_flags}"
    CACHE INTERNAL "Compile flags for ${_config} build type")

# Set linker flags
foreach(bin_type IN ITEMS EXE STATIC SHARED MODULE)
    set(CMAKE_${bin_type}_LINKER_FLAGS_${_CONFIG} "-Wl,--no-undefined -Wl,-z,max-page-size=0x1000"
        CACHE INTERNAL "Linker flags for ${bin_type} with ${_config} build type")
endforeach()

# Add the build types to the possible ones
if(CMAKE_CONFIGURATION_TYPES)
  list(APPEND CMAKE_CONFIGURATION_TYPES ${_config})
endif()

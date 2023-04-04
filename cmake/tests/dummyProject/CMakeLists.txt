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
cmake_minimum_required(VERSION 3.15)

project(FooBar
        VERSION 0.2
        LANGUAGES CXX
        DESCRIPTION "Dummy project to test gaudi_* functions")

# Include the functions
include(${CMAKE_CURRENT_LIST_DIR}/../../GaudiToolbox.cmake)

# Test gaudi_add_library
gaudi_add_library(foo SOURCES src/foo.cpp include/foo.hpp)
if(NOT TARGET foo)
    message(FATAL_ERROR "gaudi_add_library() does not create a target.")
endif()
if(NOT TARGET ${PROJECT_NAME}::foo)
    message(FATAL_ERROR "gaudi_add_library() does not create an alias target.")
endif()

# Test gaudi_add_module
add_executable(Gaudi::listcomponents IMPORTED)
set_target_properties(Gaudi::listcomponents PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/scripts/fakelistcomponents.sh")
add_executable(Gaudi::genconf IMPORTED)
set_target_properties(Gaudi::genconf PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/scripts/fakegenconf.sh")
gaudi_add_module(bar
    SOURCES src/bar.cpp
    LINK foo)
if(NOT TARGET bar)
    message(FATAL_ERROR "gaudi_add_module() does not create a target.")
endif()

# Test link
gaudi_add_library(baz
    SOURCES src/baz.cpp include/baz.hpp
    LINK PUBLIC foo)
gaudi_generate_version_header_file(baz) # with argument
target_include_directories(baz PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)
get_target_property(baz_link_lib baz LINK_LIBRARIES)
get_target_property(baz_int_link_lib baz INTERFACE_LINK_LIBRARIES)
if(NOT baz_link_lib STREQUAL foo OR NOT baz_int_link_lib STREQUAL foo)
    message(FATAL_ERROR "gaudi_add_library() does not link libraries correctly.")
endif()

# Test executable
gaudi_add_executable(qux SOURCES src/qux.cpp
    LINK baz)
if(NOT TARGET qux)
    message(FATAL_ERROR "gaudi_add_executable() does not create a target.")
endif()
gaudi_generate_version_header_file() # without argument
target_include_directories(qux PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)
get_target_property(qux_link_lib qux LINK_LIBRARIES)
get_target_property(qux_int_link_lib qux INTERFACE_LINK_LIBRARIES)
if(NOT qux_link_lib STREQUAL baz OR qux_int_link_lib)
    message(FATAL_ERROR "gaudi_add_executable() does not link libraries correctly.")
endif()

# Test python tests
enable_testing()
gaudi_add_pytest(python OPTIONS --doctest-modules)

# Install python modules
gaudi_install(PYTHON)
# Install other scripts
gaudi_install(SCRIPTS)

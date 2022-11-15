#####################################################################################
# (c) Copyright 2013-2022 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# GaudiPluginService subdirectory
# Provide a simple and generic plugin system.
# author: Marco Clemencic <marco.clemencic@cern.ch>

# Local build options
option(GAUDI_REFLEX_COMPONENT_ALIASES
       "allow use of old style (Reflex) component names"
       ON)

# Decide whether to use std::filesystem or boost::filesystem.
set( GAUDI_USE_STD_FILESYSTEM_DEFAULT FALSE )
if((("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND
     "${CMAKE_CXX_COMPILER_VERSION}" VERSION_GREATER_EQUAL 8) OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND
     "${CMAKE_CXX_COMPILER_VERSION}" VERSION_GREATER_EQUAL 7 AND
     NOT APPLE AND NOT "${CMAKE_CXX_COMPILER_VERSION}" STREQUAL "10.0.0")) AND
   "${GAUDI_CXX_STANDARD}" STRGREATER_EQUAL "17")
  set( GAUDI_USE_STD_FILESYSTEM_DEFAULT TRUE )
endif()
option( GAUDI_USE_STD_FILESYSTEM "Use std::filesystem in GaudiPluginService code"
  ${GAUDI_USE_STD_FILESYSTEM_DEFAULT} )

# Based on that figure out how to build GaudiPluginService.
if(GAUDI_USE_STD_FILESYSTEM)
  set(filesystem_implementation stdc++fs)
else()
  set(filesystem_implementation Boost::filesystem)
endif()

# Build library
gaudi_add_library(GaudiPluginService
                  SOURCES src/PluginServiceV2.cpp
                          src/PluginServiceV1.cpp
                          src/capi_pluginservice.cpp
                  LINK
                    PRIVATE ${CMAKE_DL_LIBS}
                            ${filesystem_implementation}
                    PUBLIC Threads::Threads)
if(GAUDI_REFLEX_COMPONENT_ALIASES)
  target_compile_definitions(GaudiPluginService PRIVATE
    GAUDI_REFLEX_COMPONENT_ALIASES)
endif()
if(NOT GAUDI_USE_STD_FILESYSTEM)
  target_compile_definitions(GaudiPluginService PRIVATE
    USE_BOOST_FILESYSTEM)
endif()

# Build runtime
gaudi_add_executable(listcomponents
                     SOURCES src/listcomponents.cpp
                     LINK GaudiPluginService
                          ${CMAKE_DL_LIBS})

# Install python modules
gaudi_install(PYTHON)

# Tests
if(BUILD_TESTING)
  gaudi_add_library(Test_GaudiPluginService_UseCasesLib
                    SOURCES tests/src/UseCasesLib.cpp
                    LINK PRIVATE GaudiPluginService
                        Boost::unit_test_framework)
  gaudi_add_executable(Test_GaudiPluginService_UseCases
                      SOURCES tests/src/UseCasesTests.cpp
                      LINK GaudiPluginService
                            Boost::unit_test_framework
                      TEST)

  gaudi_add_library(Test_GaudiPluginService_LegacyUseCasesLib
                    SOURCES tests/src/LegacyUseCasesLib.cpp
                    LINK PRIVATE GaudiPluginService)
  gaudi_add_executable(Test_GaudiPluginService_LegacyUseCases
                      SOURCES tests/src/LegacyUseCasesTests.cpp
                      LINK GaudiPluginService
                            Boost::unit_test_framework
                      TEST)

  get_filename_component(package_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  # tests on listcomponents
  add_test(NAME ${package_name}.listcomponents.usage
          COMMAND run $<TARGET_FILE:listcomponents>)
  set_tests_properties(${package_name}.listcomponents.usage
                      PROPERTIES PASS_REGULAR_EXPRESSION "Usage:")
  add_test(NAME ${package_name}.listcomponents.help1
          COMMAND run $<TARGET_FILE:listcomponents> -h)
  set_tests_properties(${package_name}.listcomponents.help1
                      PROPERTIES PASS_REGULAR_EXPRESSION "Options:")
  add_test(NAME ${package_name}.listcomponents.help2
          COMMAND run $<TARGET_FILE:listcomponents> --help)
  set_tests_properties(${package_name}.listcomponents.help2
                      PROPERTIES PASS_REGULAR_EXPRESSION "Options:")
  add_test(NAME ${package_name}.listcomponents.wrong_args
          COMMAND run $<TARGET_FILE:listcomponents> -o)
  set_tests_properties(${package_name}.listcomponents.wrong_args
                      PROPERTIES PASS_REGULAR_EXPRESSION
                                  "ERROR: missing argument")
  add_test(NAME ${package_name}.listcomponents.v1
          COMMAND run $<TARGET_FILE:listcomponents> $<TARGET_FILE_NAME:Test_GaudiPluginService_LegacyUseCasesLib>)
  set_tests_properties(${package_name}.listcomponents.v1
    PROPERTIES PASS_REGULAR_EXPRESSION "v1::$<TARGET_FILE_NAME:Test_GaudiPluginService_LegacyUseCasesLib>:Test::ComponentA")
  add_test(NAME ${package_name}.listcomponents.v2
          COMMAND run $<TARGET_FILE:listcomponents> $<TARGET_FILE_NAME:Test_GaudiPluginService_UseCasesLib>)
  set_tests_properties(${package_name}.listcomponents.v2
    PROPERTIES PASS_REGULAR_EXPRESSION "v2::$<TARGET_FILE_NAME:Test_GaudiPluginService_UseCasesLib>:special-id")
endif()

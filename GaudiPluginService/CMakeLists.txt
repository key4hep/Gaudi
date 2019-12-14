#####################################################################################
# (c) Copyright 1998-2019 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# Provide a simple and generic plugin system.
# author: Marco Clemencic <marco.clemencic@cern.ch>

gaudi_subdir(GaudiPluginService)

# Local build options
option(GAUDI_REFLEX_COMPONENT_ALIASES
       "allow use of old style (Reflex) component names"
       ON)
if(GAUDI_REFLEX_COMPONENT_ALIASES)
  add_definitions(-DGAUDI_REFLEX_COMPONENT_ALIASES)
endif()

# Decide whether to use std::filesystem or boost::filesystem.
set( GAUDI_USE_STD_FILESYSTEM_DEFAULT FALSE )
if((("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND
     "${CMAKE_CXX_COMPILER_VERSION}" VERSION_GREATER_EQUAL 8) OR
    ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND
     "${CMAKE_CXX_COMPILER_VERSION}" VERSION_GREATER_EQUAL 7 AND
     NOT APPLE)) AND
   "${GAUDI_CXX_STANDARD}" STRGREATER_EQUAL "c++17")
  set( GAUDI_USE_STD_FILESYSTEM_DEFAULT TRUE )
endif()
option( GAUDI_USE_STD_FILESYSTEM "Use std::filesystem in the Gaudi code"
  ${GAUDI_USE_STD_FILESYSTEM_DEFAULT} )

# Based on that figure out how to build GaudiPluginService.
set(includes)
set(libs)
set(defines)
if(GAUDI_USE_STD_FILESYSTEM)
  list(APPEND libs -lstdc++fs)
else()
  find_package(Boost COMPONENTS filesystem REQUIRED)
  list(APPEND includes INCLUDE_DIRS Boost)
  list(APPEND libs Boost)
  list(APPEND defines -DUSE_BOOST_FILESYSTEM)
endif()

# Library
gaudi_add_library(GaudiPluginService src/PluginServiceV2.cpp src/PluginServiceV1.cpp src/capi_pluginservice.cpp
                  ${includes}
                  LINK_LIBRARIES ${CMAKE_DL_LIBS} ${libs}
                  PUBLIC_HEADERS Gaudi)
if(defines)
  target_compile_definitions(GaudiPluginService PRIVATE ${defines})
endif()

# Disable generation of ConfUserDB (must be done before gaudi_install_python_modules)
set_directory_properties(PROPERTIES CONFIGURABLE_USER_MODULES None)
# python modules
gaudi_install_python_modules()

# Application
gaudi_add_executable(listcomponents src/listcomponents.cpp)
target_link_libraries(listcomponents GaudiPluginService ${CMAKE_DL_LIBS})

# Hide some Boost compile time warnings
find_package(Boost REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

gaudi_add_library(Test_GaudiPluginService_UseCasesLib tests/src/UseCasesLib.cpp
                  NO_PUBLIC_HEADERS
                  LINK_LIBRARIES GaudiPluginService)
gaudi_add_unit_test(Test_GaudiPluginService_UseCases tests/src/UseCasesTests.cpp
                    LINK_LIBRARIES GaudiPluginService
                    TYPE Boost)

gaudi_add_library(Test_GaudiPluginService_LegacyUseCasesLib tests/src/LegacyUseCasesLib.cpp
                  NO_PUBLIC_HEADERS
                  LINK_LIBRARIES GaudiPluginService)
gaudi_add_unit_test(Test_GaudiPluginService_LegacyUseCases tests/src/LegacyUseCasesTests.cpp
                    LINK_LIBRARIES GaudiPluginService Boost
                    TYPE Boost)

gaudi_add_test(listcomponents.usage
               COMMAND $<TARGET_FILE_NAME:listcomponents>
               PASSREGEX "Usage:")
gaudi_add_test(listcomponents.help1
               COMMAND $<TARGET_FILE_NAME:listcomponents> -h
               PASSREGEX "Options:")
gaudi_add_test(listcomponents.help2
               COMMAND $<TARGET_FILE_NAME:listcomponents> --help
               PASSREGEX "Options:")
gaudi_add_test(listcomponents.wrong_args
               COMMAND $<TARGET_FILE_NAME:listcomponents> -o
               PASSREGEX "ERROR: missing argument")

gaudi_add_test(listcomponents.v1 COMMAND $<TARGET_FILE_NAME:listcomponents> $<TARGET_FILE_NAME:Test_GaudiPluginService_LegacyUseCasesLib>
               PASSREGEX "v1::$<TARGET_FILE_NAME:Test_GaudiPluginService_LegacyUseCasesLib>:Test::ComponentA")
gaudi_add_test(listcomponents.v2 COMMAND $<TARGET_FILE_NAME:listcomponents> $<TARGET_FILE_NAME:Test_GaudiPluginService_UseCasesLib>
               PASSREGEX "v2::$<TARGET_FILE_NAME:Test_GaudiPluginService_UseCasesLib>:special-id")

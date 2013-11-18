CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)

# Ensure that we can find GaudiProjectConfig.cmake
# (this works only for projects embedding GaudiProjectConfig.cmake)
if(NOT GaudiProject_DIR AND ("$ENV{GaudiProject_DIR}" STREQUAL ""))
  set(GaudiProject_DIR ${CMAKE_SOURCE_DIR}/cmake)
endif()

#---------------------------------------------------------------
# Load macros and functions for Gaudi-based projects
find_package(GaudiProject)
#---------------------------------------------------------------

# Declare project name and version
gaudi_project(Gaudi v24r1)

# This test does not really fit in a subdirectory.
gaudi_add_test(cmake.EnvConfigTests
               COMMAND nosetests ${CMAKE_SOURCE_DIR}/cmake/EnvConfig)

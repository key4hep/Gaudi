CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/cmake/modules  ${CMAKE_MODULE_PATH})

# Ensure that we can find GaudiprojectConfig.cmake
if(NOT GaudiProject_DIR AND ("$ENV{GaudiProject_DIR}" STREQUAL ""))
  set(GaudiProject_DIR ${CMAKE_SOURCE_DIR}/cmake)
endif()

#---------------------------------------------------------------
# Load macros and functions for Gaudi-based projects
find_package(GaudiProject)
#---------------------------------------------------------------

# Declare project name and version
gaudi_project(Gaudi v23r4)

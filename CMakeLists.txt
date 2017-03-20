CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)

# Configurable options for building Gaudi in the ATLAS software stack:
option( GAUDI_ATLAS "Enable ATLAS-specific settings" OFF )
set( GAUDI_ATLAS_BASE_PROJECT "AtlasExternals" CACHE STRING
   "Name of the ATLAS base project to build Gaudi against" )

# Ensure that we can find GaudiProjectConfig.cmake
# (this works only for projects embedding GaudiProjectConfig.cmake)
if(NOT GaudiProject_DIR AND ("$ENV{GaudiProject_DIR}" STREQUAL ""))
  set(GaudiProject_DIR ${CMAKE_SOURCE_DIR}/cmake)
endif()

#---------------------------------------------------------------
# Load macros and functions for Gaudi-based projects
find_package(GaudiProject)
#---------------------------------------------------------------

# Base Gaudi on "the ATLAS base project" (AtlasExternals by default) when
# building for ATLAS:
if( GAUDI_ATLAS )
   find_package( ${GAUDI_ATLAS_BASE_PROJECT} )
   gaudi_ctest_setup() # Needed for correct CTest usage in this setup
endif()

# Find/set up some needed externals:
include(${CMAKE_SOURCE_DIR}/cmake/externals.cmake)

# Declare project name and version
gaudi_project(Gaudi v28r2)

# These tests do not really fit in a subdirectory.
gaudi_add_test(cmake.EnvConfigTests
               COMMAND nosetests --with-doctest ${CMAKE_SOURCE_DIR}/cmake/EnvConfig)
gaudi_add_test(cmake.CMakeModules
               COMMAND nosetests ${CMAKE_SOURCE_DIR}/cmake/tests)
gaudi_add_test(cmake.QMTDeps
               COMMAND nosetests --with-doctest ${CMAKE_SOURCE_DIR}/cmake/extract_qmtest_metadata.py)

# Configure how CPack should run:
find_file( _cpack_config NAMES GaudiCPackSettings.cmake
   PATHS ${CMAKE_MODULE_PATH} )
if( _cpack_config )
   include( ${_cpack_config} )
else()
   message( WARNING "Could not find GaudiCPackSettings.cmake" )
endif()
unset( _cpack_config )
mark_as_advanced( _cpack_config )

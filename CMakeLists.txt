CMAKE_MINIMUM_REQUIRED(VERSION 2.8.5)

# Ensure that we can find GaudiProjectConfig.cmake
# (this works only for projects embedding GaudiProjectConfig.cmake)
if(NOT GaudiProject_DIR AND ("$ENV{GaudiProject_DIR}" STREQUAL ""))
  set(GaudiProject_DIR ${CMAKE_SOURCE_DIR}/cmake)
endif()


option(GAUDI_USE_SYSTEM_CPP_GSL
       "If to use the C++ Guidelines Support Library from the system or providing it through Gaudi"
       NO)
if(NOT GAUDI_USE_SYSTEM_CPP_GSL)
  set(CPP_GSL_URL https://github.com/Microsoft/GSL)
  set(CPP_GSL_VERSION 32ca283d)
  message(STATUS "Using and shipping ${CPP_GSL_URL} version ${CPP_GSL_VERSION}")
  if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/GSL)
    execute_process(COMMAND git clone ${CPP_GSL_URL})
  endif()
  execute_process(COMMAND git checkout ${CPP_GSL_VERSION}
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/GSL)
  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/GSL/gsl DESTINATION include)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/GSL/LICENSE DESTINATION include/gsl/LICENSE)
  set(CPP_GSL_INCLUDE_DIR NAMES ${CMAKE_CURRENT_BINARY_DIR}/GSL)
else()
  find_path(CPP_GSL_INCLUDE_DIR NAMES gsl/gsl)
endif()
if(CPP_GSL_INCLUDE_DIR)
  set(CPP_GSL_INCLUDE_DIRS ${CPP_GSL_INCLUDE_DIR})
  include_directories(SYSTEM ${CPP_GSL_INCLUDE_DIRS})
  set(CPP_GSL_FOUND TRUE)
else()
  message(WARNING "C++ Guidelines Support Library not available")
  set(CPP_GSL_FOUND FALSE)
endif()


#---------------------------------------------------------------
# Load macros and functions for Gaudi-based projects
find_package(GaudiProject)
#---------------------------------------------------------------

option( GAUDI_ATLAS "Enable ATLAS-specific settings" OFF )
set( GAUDI_ATLAS_BASE_PROJECT "AtlasExternals" CACHE STRING
   "Name of the ATLAS base project to build Gaudi against" )

# Base Gaudi on "the ATLAS base project" (AtlasExternals by default) when
# building for ATLAS:
if( GAUDI_ATLAS )
   find_package( ${GAUDI_ATLAS_BASE_PROJECT} )
   gaudi_ctest_setup() # Needed for correct CTest usage in this setup
endif()

# Declare project name and version
gaudi_project(Gaudi v27r1)

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

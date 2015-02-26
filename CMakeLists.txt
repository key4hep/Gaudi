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

option(GAUDI_ATLAS "Enable ATLAS-specific settings" OFF)

# Declare project name and version
gaudi_project(Gaudi v26r1)

# These tests do not really fit in a subdirectory.
gaudi_add_test(cmake.EnvConfigTests
               COMMAND nosetests --with-doctest ${CMAKE_SOURCE_DIR}/cmake/EnvConfig)
gaudi_add_test(cmake.project_manifest.doctest
               COMMAND ${PYTHON_EXECUTABLE} -m doctest
                       ${CMAKE_SOURCE_DIR}/GaudiPolicy/scripts/project_manifest.py)
gaudi_add_test(cmake.CMakeModules
               COMMAND nosetests ${CMAKE_SOURCE_DIR}/cmake/tests)
gaudi_add_test(cmake.QMTDeps
               COMMAND nosetests --with-doctest ${CMAKE_SOURCE_DIR}/cmake/extract_qmtest_metadata.py)

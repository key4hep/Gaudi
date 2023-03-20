#####################################################################################
# (c) Copyright 1998-2023 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
#[========================================================================[.rst:
Gaudi
-----

This file is the top level CMakeLists.txt that describes the whole
configuration of the build of Gaudi.

A few commands
^^^^^^^^^^^^^^

* Configure: ``cmake -S . -B build.$BINARY_TAG -D CMAKE_BUILD_TYPE=Developer [-G Ninja]``
* Compile: ``cmake --build build.$BINARY_TAG -j `nproc` ``
* Run tests: ``cd build.$BINARY_TAG ; ctest -j `nproc` ; cd ..``
* Install: ``cmake --install build.$BINARY_TAG [--prefix path/to/where/you/want]``
* Package (zip+rpm): ``cmake --build build.$BINARY_TAG -t package``

List of available options
^^^^^^^^^^^^^^^^^^^^^^^^^

An option is a cached variable that can be turned ON or OFF.
To get descriptions and values of these options:
  ``cmake -N -LH build.$BINARY_TAG``
after the configuration or use ccmake or cmake-gui.

* Optional dependencies
  * GAUDI_USE_AIDA
  * GAUDI_USE_XERCESC
  * GAUDI_USE_CLHEP
  * GAUDI_USE_HEPPDT
  * GAUDI_USE_CPPUNIT
  * GAUDI_USE_UNWIND
  * GAUDI_USE_GPERFTOOLS
  * GAUDI_USE_DOXYGEN
  * GAUDI_USE_INTELAMPLIFIER
  * GAUDI_USE_JEMALLOC
* Install options
  * GAUDI_INSTALL_OPTIONAL
* Install layout variables
  * CMAKE_INSTALL_BINDIR
  * CMAKE_INSTALL_LIBDIR
  * CMAKE_INSTALL_INCLUDEDIR
  * GAUDI_INSTALL_PLUGINDIR
  * GAUDI_INSTALL_PYTHONDIR
  * GAUDI_INSTALL_CONFIGDIR
* For sanitized build
  * GAUDI_GENCONF_NO_FAIL
* Compile options
  * GAUDI_REFLEX_COMPONENT_ALIASES
* Doxygen
  * DOXYGEN_WITH_LOCAL_MATHJAX
  * DOXYGEN_WITH_CPPREFERENCE_LINKS

To disable the build of tests, set the option ``BUILD_TESTING`` to FALSE.

When to re-configure the project manually?
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

* When a new .qmt file is added.
* When a new python package is added in a python/ folder that already
  had one installed python package.
* When a new release.notes*.html is added to GaudiRelease/doc/.

TWiki documentation
^^^^^^^^^^^^^^^^^^^

Developer's guide: `<https://twiki.cern.ch/twiki/bin/view/LHCb/GaudiCMake315Configuration>`_
Maintainer's guide: `<https://twiki.cern.ch/twiki/bin/view/LHCb/MaintainGaudiCMake315Configuration>`_

#]========================================================================]

cmake_minimum_required(VERSION 3.15)

project(Gaudi VERSION 36.12
              LANGUAGES CXX
              DESCRIPTION "Gaudi Software Framework"
              HOMEPAGE_URL "https://cern.ch/gaudi")

# Add the "Developer" build type
include(cmake/DeveloperBuildType.cmake)

# Set up the GAUDI_ATOMIC_LIBS variable
include(cmake/GaudiAtomicLibs.cmake)

# Import Gaudi functions (gaudi_*) ==> for documentation look in the file
include(cmake/GaudiToolbox.cmake)

# Export the list of compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "Enable/Disable output of compile_commands.json" FORCE)

set(GAUDI_USE_PYTHON_MAJOR 3 CACHE STRING "Major version of Python to use")
set(GAUDI_OPTIONAL_DEPENDENCIES "${GAUDI_OPTIONAL_DEPENDENCIES}set(GAUDI_USE_PYTHON_MAJOR ${GAUDI_USE_PYTHON_MAJOR})\n")

# Find all the dependencies of the project
list(PREPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules") # (Find*.cmake)
include(cmake/GaudiDependencies.cmake)

# The C++ standard used must be the same as ROOT's
if(NOT GAUDI_CXX_STANDARD)
  # FIXME: the day ROOT_CXX_STANDARD is provided by ROOTConfig.cmake, remove this lookup
  set(ROOT_CXX_STANDARD ${ROOT_CXX_FLAGS}) # try to parse ROOT compile flags
  separate_arguments(ROOT_CXX_STANDARD)
  list(FILTER ROOT_CXX_STANDARD INCLUDE REGEX "-std=(c|gnu)\\+\\+[0-9]+")
  list(TRANSFORM ROOT_CXX_STANDARD REPLACE "-std=(c|gnu)\\+\\+([0-9]+)" "\\2")
  if(NOT ROOT_CXX_STANDARD)
    message(FATAL_ERROR "Cannot find the C++ standard of ROOT")
  endif()
  list(GET ROOT_CXX_STANDARD 0 ROOT_CXX_STANDARD)
  set(GAUDI_CXX_STANDARD ${ROOT_CXX_STANDARD} CACHE STRING "The version of C++ used to compile Gaudi")
endif()
message(STATUS "Gaudi will be built using C++${GAUDI_CXX_STANDARD} standard.")
set(CMAKE_CXX_STANDARD ${GAUDI_CXX_STANDARD})
set(CMAKE_CXX_STANDARD_REQUIRED TRUE) # do not fall back on older version
set(CMAKE_CXX_EXTENSIONS OFF) # strict c++.. instead of gnu++..

# Enable testing with CTest/CDash
include(CTest)

# These are test-only, private dependencies
if(BUILD_TESTING)
  find_package(Catch2 REQUIRED)
  include(Catch)
endif()

# Include sub-projects one by one without introducing dependency conflicts
add_subdirectory(GaudiPluginService)
add_subdirectory(GaudiPolicy)
add_subdirectory(GaudiKernel)
add_subdirectory(GaudiConfiguration)
add_subdirectory(GaudiCoreSvc)
add_subdirectory(GaudiUtils)
add_subdirectory(Gaudi)
add_subdirectory(GaudiAlg)
add_subdirectory(GaudiAud)
add_subdirectory(GaudiCommonSvc)
add_subdirectory(GaudiHive)
add_subdirectory(GaudiMonitor)
add_subdirectory(GaudiMP)
add_subdirectory(GaudiPartProp)
add_subdirectory(GaudiProfiling)
add_subdirectory(GaudiPython)
add_subdirectory(GaudiRelease)
add_subdirectory(GaudiSvc)
add_subdirectory(PartPropSvc)
add_subdirectory(RootCnv)
add_subdirectory(RootHistCnv)
add_subdirectory(GaudiExamples)

# Generate GAUDI_VERSION.h
gaudi_generate_version_header_file()

# Framework level tests
# Test scripts inside cmake/
gaudi_add_tests(nosetests ${PROJECT_SOURCE_DIR}/cmake/extract_qmtest_metadata.py)

if(NOT PROJECT_NAME STREQUAL CMAKE_PROJECT_NAME) # If we are building the full stack as one project
  # we do not install nor package nor register CMake tests
  # but we add Gaudi's CMake modules to the CMAKE_MODULE_PATH for downstream projects
  list(PREPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} PARENT_SCOPE)
  return()
endif()

# Unit tests of gaudi_* functions
include(cmake/tests/testGaudiToolbox.cmake)
# Unit tests of the installation
include(cmake/tests/testGaudiInstallation.cmake)
# Unit tests of Gaudi as dependency in a downstream project (with config files)
include(cmake/tests/testGaudiDownstream.cmake)

# Installation of Gaudi
include(CMakePackageConfigHelpers)
# Generate the config files
configure_package_config_file(cmake/GaudiConfig.cmake.in ${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION "${GAUDI_INSTALL_CONFIGDIR}"
  PATH_VARS CMAKE_INSTALL_BINDIR CMAKE_INSTALL_LIBDIR CMAKE_INSTALL_INCLUDEDIR
            GAUDI_INSTALL_PLUGINDIR GAUDI_INSTALL_PYTHONDIR
  NO_CHECK_REQUIRED_COMPONENTS_MACRO)
write_basic_package_version_file(${PROJECT_NAME}ConfigVersion.cmake
  COMPATIBILITY AnyNewerVersion)
# Install the set of exported targets
install(EXPORT ${PROJECT_NAME} NAMESPACE ${PROJECT_NAME}::
            DESTINATION "${CMAKE_INSTALL_PREFIX}"
        FILE "${PROJECT_NAME}Targets.cmake"
            DESTINATION "${GAUDI_INSTALL_CONFIGDIR}")
# Install cmake files for downstream project to be able to use Gaudi
gaudi_install(CMAKE cmake/GaudiToolbox.cmake
                    cmake/header_build_test.tpl
                    cmake/GaudiDependencies.cmake
                    cmake/DeveloperBuildType.cmake
                    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
                    cmake/modules) # Find*.cmake

install(PROGRAMS
        cmake/extract_qmtest_metadata.py # used in gaudi_add_tests(QMTest)
        cmake/scan_dict_deps.py # used in gaudi_add_dictionary(...)
    DESTINATION "${GAUDI_INSTALL_CONFIGDIR}"
)

# Add an uninstall target
add_custom_target(uninstall
  COMMAND test -f ${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt &&
          xargs rm < ${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt &&
          rm ${CMAKE_CURRENT_BINARY_DIR}/install_manifest.txt &&
          echo ${PROJECT_NAME} uninstalled successfully. ||
          echo ${PROJECT_NAME} has not been installed yet or you need to be root to uninstall.
  COMMENT "Uninstalling ${PROJECT_NAME}")

# CPack configuration
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "CERN-LHCb")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
foreach(v IN ITEMS MAJOR MINOR PATCH)
   set(CPACK_PACKAGE_VERSION_${v} ${PROJECT_VERSION_${v}})
endforeach()
if(DEFINED BINARY_TAG)
  set(CPACK_SYSTEM_NAME ${BINARY_TAG})
elseif(DEFINED ENV{BINARY_TAG})
  set(CPACK_SYSTEM_NAME $ENV{BINARY_TAG})
endif()
set(CPACK_GENERATOR "ZIP;RPM") # package format
# The following line is needed to prevent the compilation of python modules before packaging (an incorrect version of python may be used).
# FIXME: the day we can tell CMake to prepend the call to rpm by the run script, do it and remove this line (something like ``set(CPACK_RPM_LAUNCHER ${CMAKE_BINARY_DIR}/run)``)
set(CPACK_RPM_SPEC_MORE_DEFINE "%global __os_install_post %(echo '%{__os_install_post}' | sed -e 's!/usr/lib[^[:space:]]*/brp-python-bytecompile[[:space:]].*$!!g')")
# -- for the sources
set(CPACK_SOURCE_IGNORE_FILES "/InstallArea/;/build\\\\..*/;/\\\\.git/;/\\\\.git.*")
set(CPACK_SOURCE_GENERATOR "ZIP;RPM")
include(CPack)

# Set the version of the project as a cache variable to be seen by other
# projects in the same super-project.
set(${PROJECT_NAME}_VERSION "${PROJECT_VERSION}" CACHE STRING "Version of ${PROJECT_NAME}" FORCE)

# Optionally enable compatibility with old-style CMake configurations, via helper module
option(GAUDI_LEGACY_CMAKE_SUPPORT "Enable compatibility with old-style CMake builds" "$ENV{GAUDI_LEGACY_CMAKE_SUPPORT}")
if(GAUDI_LEGACY_CMAKE_SUPPORT)
  find_file(legacy_cmake_config_support NAMES LegacyGaudiCMakeSupport.cmake)
  if(legacy_cmake_config_support)
    include(${legacy_cmake_config_support})
  else()
    message(FATAL_ERROR "GAUDI_LEGACY_CMAKE_SUPPORT set to TRUE, but cannot find LegacyGaudiCMakeSupport.cmake")
  endif()
endif()

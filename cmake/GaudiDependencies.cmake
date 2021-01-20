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
GaudiDependencies
-----------------

This file gather all the calls to find_package() used by Gaudi to find all
the third party libraries needed to build it.

Why a separate file?
^^^^^^^^^^^^^^^^^^^^

This dependency lookup is in a separate file so that it can be installed.
This way it can be included by GaudiConfig.cmake to perform the same lookup in
downstream projects.

Usage
^^^^^

First include the file GaudiToolbox.cmake and set the options GAUDI_USE_*,
then include this file.
It will search the dependencies with respect to these options.

If you are building Gaudi as a sub-project of a stack of projects,
remember that find_package() is local to the project, the third
party libraries searched in this file will not be available in other
projects of the stack.
.. code-block:: cmake

    if(NOT TARGET Gaudi::GaudiPluginService) # (because targets are global)
      find_package(Gaudi REQUIRED) # Imports targets, functions and dependencies of Gaudi
    endif()
    find_package(...) # other dependencies of the project

What is searched?
^^^^^^^^^^^^^^^^^

* Boost>=1.70
* Python>=2.7.15
* ROOT>=6.18
* UUID
* TBB>=2019.0.11007.2
* Threads
* ZLIB>=1.2.11
* Rangev3
* nosetests

Depending on what was used to build Gaudi:
* AIDA
* XercesC
* CLHEP>=2.4.1.0
* HepPDT
* CppUnit
* unwind
* gperftools>=2.7.0
* Doxygen>=1.8.15
* IntelAmplifier
* jemalloc

Depending on the platform:
* Linux
  - rt
* MacOS
  - Foundation

Variables to modify the configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Set this variable to TRUE before including this file:

* ``GAUDI_DEPENDENCIES_FIND_QUIETLY``: pass QUIET to all calls to find_package()
  NB: It has no effect on Boost because its config file is badly written.

#]========================================================================]

set(__quiet)
if(GAUDI_DEPENDENCIES_FIND_QUIETLY)
  set(__quiet QUIET)
endif()

include(FeatureSummary)
set_property(GLOBAL PROPERTY FeatureSummary_REQUIRED_PKG_TYPES RECOMMENDED REQUIRED)

# Note: this must be set before the first `find_package`
set(THREADS_PREFER_PTHREAD_FLAG YES)

set(Boost_USE_STATIC_LIBS OFF)
set(OLD_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS}) # FIXME: the day BoostConfig.cmake handles Boost_USE_STATIC_LIBS correctly
set(BUILD_SHARED_LIBS ON)
find_package(Boost 1.70 ${__quiet} CONFIG COMPONENTS system filesystem regex
  thread python unit_test_framework program_options log log_setup graph)
set_package_properties(Boost PROPERTIES TYPE REQUIRED)
set(BUILD_SHARED_LIBS ${OLD_BUILD_SHARED_LIBS})
mark_as_advanced(Boost_DIR) # FIXME: the day Boost correctly marks as advanced its variables
foreach(component IN ITEMS system filesystem regex thread python unit_test_framework
                           program_options log log_setup graph atomic chrono
                           date_time headers)
  mark_as_advanced(boost_${component}_DIR)
endforeach()

if(GAUDI_USE_PYTHON_MAJOR STREQUAL "2")
  set(_gaudi_Python_MIN_VERSION 2.7.15)
elseif(GAUDI_USE_PYTHON_MAJOR STREQUAL "3")
  set(_gaudi_Python_MIN_VERSION 3.7)
else()
  message(FATAL_ERROR "Invalid value for GAUDI_USE_PYTHON_MAJOR (${GAUDI_USE_PYTHON_MAJOR}), only 2 and 3 are supported")
endif()
find_package(Python ${_gaudi_Python_MIN_VERSION} ${__quiet} COMPONENTS Interpreter Development)
set_package_properties(Python PROPERTIES TYPE REQUIRED)

find_package(ROOT 6.18 ${__quiet} CONFIG COMPONENTS Core RIO Hist Thread Matrix
  MathCore Net XMLIO Tree TreePlayer Graf3d Graf Gpad)
mark_as_advanced(ROOT_DIR ROOT_genmap_CMD ROOT_rootdraw_CMD)
set_package_properties(ROOT PROPERTIES TYPE REQUIRED)
# FIXME: the day ROOTConfig.cmake displays at least that it was found remove these lines
if(NOT GAUDI_DEPENDENCIES_FIND_QUIETLY)
  message(STATUS "Found ROOT: ${ROOT_DIR} (found version ${ROOT_VERSION})")
endif()

# FIXME: if you are using the normal version of TBB, it has a config file so
#   remove cmake/FindTBB.cmake and add CONFIG to the next line.
find_package(TBB 2019.0.11007.2 ${__quiet})
set_package_properties(TBB PROPERTIES TYPE REQUIRED)

set(_gaudi_ZLIB_MIN_VERSION 1.2.11)
foreach(dep IN ITEMS UUID Threads ZLIB Rangev3 cppgsl fmt nlohmann_json)
  find_package(${dep} ${_gaudi_${dep}_MIN_VERSION} ${__quiet})
  set_package_properties(${dep} PROPERTIES TYPE REQUIRED)
endforeach()

set(_gaudi_CLHEP_MIN_VERSION 2.4.0.1)
set(_gaudi_gperftools_MIN_VERSION 2.7.0)
set(_gaudi_Doxygen_MIN_VERSION 1.8.15)
set(CLHEP_FORCE_MODE CONFIG)
foreach(dep IN ITEMS AIDA XercesC CLHEP HepPDT CppUnit unwind gperftools
                     Doxygen IntelAmplifier jemalloc)
  string(TOUPPER ${dep} DEP)
  if(GAUDI_USE_${DEP})
    find_package(${dep} ${_gaudi_${dep}_MIN_VERSION} ${${dep}_FORCE_MODE} ${__quiet})
    if(CMAKE_FIND_PACKAGE_NAME) # if the lookup is performed from GaudiConfig.cmake
      # then, all optional dependencies become required
      set_package_properties(${dep} PROPERTIES TYPE REQUIRED)
    else()
      set_package_properties(${dep} PROPERTIES TYPE RECOMMENDED)
    endif()
  endif()
endforeach()

# Make sure platform specific system libraries are available
if(UNIX AND NOT APPLE)
  find_library(rt_LIBRARY rt)
  mark_as_advanced(rt_LIBRARY)
  if(NOT rt_LIBRARY)
    message(FATAL_ERROR "rt was not found")
  endif()
endif()

if(APPLE)
  find_library(Foundation_FRAMEWORK Foundation)
  mark_as_advanced(Foundation_FRAMEWORK)
  if(NOT Foundation_FRAMEWORK)
    message(FATAL_ERROR "Foundation framework was not found")
  endif()
endif()

# Print a summary of the lookup
feature_summary(FATAL_ON_MISSING_REQUIRED_PACKAGES
  DESCRIPTION "Missing Gaudi's required dependencies:"
  QUIET_ON_EMPTY
  WHAT REQUIRED_PACKAGES_NOT_FOUND)
feature_summary(DESCRIPTION "Gaudi's required optional dependencies found:"
  QUIET_ON_EMPTY
  WHAT RECOMMENDED_PACKAGES_FOUND)
feature_summary(FATAL_ON_MISSING_REQUIRED_PACKAGES
  DESCRIPTION "Missing Gaudi's required optional dependencies:"
  QUIET_ON_EMPTY
  WHAT RECOMMENDED_PACKAGES_NOT_FOUND)

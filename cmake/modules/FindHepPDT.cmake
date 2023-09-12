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
# - Locate HepPDT library
# Defines:
#
#  HEPPDT_FOUND
#  HEPPDT_INCLUDE_DIR
#  HEPPDT_INCLUDE_DIRS (not cached)
#  HEPPDT_PDT_LIBRARY
#  HEPPDT_PID_LIBRARY
#  HEPPDT_LIBRARIES (not cached)
#
# Imports:
#
#  HepPDT::heppdt
#
# Usage of the target instead of the variables is advised

# Find quietly if already found before
if(DEFINED CACHE{HEPPDT_INCLUDE_DIR})
  set(${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY YES)
endif()

find_path(HEPPDT_INCLUDE_DIR HepPDT/ParticleData.hh
          HINTS $ENV{HEPPDT_ROOT_DIR}/include ${HEPPDT_ROOT_DIR}/include)
set(HEPPDT_VERSION 0.0.0)
if (HEPPDT_INCLUDE_DIR)
  if (EXISTS ${HEPPDT_INCLUDE_DIR}/HepPDT/defs.h)
    file(STRINGS ${HEPPDT_INCLUDE_DIR}/HepPDT/defs.h HEPPDT_VERSION_STRING_CONTENT REGEX "^#define[ ]+PACKAGE_VERSION[ ]+\"" )
    if (HEPPDT_VERSION_STRING_CONTENT)
      string(REGEX MATCH "[1234567890\.]+[a-zA-Z]*" HEPPDT_VERSION ${HEPPDT_VERSION_STRING_CONTENT})
    endif()
  endif()
endif()

find_library(HEPPDT_PDT_LIBRARY NAMES HepPDT
          HINTS $ENV{HEPPDT_ROOT_DIR}/lib HINTS $ENV{HEPPDT_ROOT_DIR}/lib64 ${HEPPDT_ROOT_DIR}/lib ${HEPPDT_ROOT_DIR}/lib64)
find_library(HEPPDT_PID_LIBRARY NAMES HepPID
          HINTS $ENV{HEPPDT_ROOT_DIR}/lib $ENV{HEPPDT_ROOT_DIR}/lib64 ${HEPPDT_ROOT_DIR}/lib ${HEPPDT_ROOT_DIR}/lib64)

set(HEPPDT_LIBRARIES ${HEPPDT_PDT_LIBRARY} ${HEPPDT_PID_LIBRARY})

set(HEPPDT_INCLUDE_DIRS ${HEPPDT_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set HEPPDT_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HepPDT REQUIRED_VARS HEPPDT_INCLUDE_DIR HEPPDT_LIBRARIES 
                                  VERSION_VAR HEPPDT_VERSION)

mark_as_advanced(HEPPDT_FOUND HEPPDT_INCLUDE_DIR HEPPDT_LIBRARIES
                 HEPPDT_PDT_LIBRARY HEPPDT_PID_LIBRARY HEPPDT_VERSION)

# Modernisation: create an interface target to link against
if(TARGET HepPDT::heppdt)
    return()
endif()
if(HEPPDT_FOUND)
  add_library(HepPDT::heppdt IMPORTED INTERFACE)
  target_include_directories(HepPDT::heppdt SYSTEM INTERFACE "${HEPPDT_INCLUDE_DIRS}")
  target_link_libraries(HepPDT::heppdt INTERFACE "${HEPPDT_LIBRARIES}")
  if (${HEPPDT_VERSION} VERSION_GREATER_EQUAL "3")
    set_property(TARGET HepPDT::heppdt APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS HEPPDT_VERSION_MAJOR=3)
  else()
    set_property(TARGET HepPDT::heppdt APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS HEPPDT_VERSION_MAJOR=2)
  endif()
  # Display the imported target for the user to know
  if(NOT ${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
    message(STATUS "  Import target: HepPDT::heppdt")
  endif()
endif()

if(COMMAND __deprecate_var_for_target)
  foreach(v IN ITEMS HEPPDT_INCLUDE_DIR HEPPDT_INCLUDE_DIRS HEPPDT_PDT_LIBRARY HEPPDT_PID_LIBRARY HEPPDT_LIBRARIES)
    variable_watch(${v} __deprecate_var_for_target)
  endforeach()
endif()

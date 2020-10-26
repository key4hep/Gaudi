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
# Module locating the C++ Guidelines Support Library headers.
#
# Defines:
#  - CPPGSL_FOUND
#  - CPPGSL_INCLUDE_DIR
#  - CPPGSL_INCLUDE_DIRS (not cached)
#
# Imports:
#
#  cppgsl::cppgsl
#
# Usage of the target instead of the variables is advised

# Find quietly if already found before
if(DEFINED CACHE{CPPGSL_INCLUDE_DIR})
  set(${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY YES)
endif()

# Look for the header directory:
find_path(CPPGSL_INCLUDE_DIR
   NAMES gsl/gsl
   HINTS $ENV{CPPGSL_ROOT_DIR} ${CPPGSL_ROOT_DIR})
set(CPPGSL_INCLUDE_DIRS ${CPPGSL_INCLUDE_DIR})

# Handle the regular find_package arguments:
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(cppgsl DEFAULT_MSG CPPGSL_INCLUDE_DIR)

# Mark the cached variables as "advanced":
mark_as_advanced(CPPGSL_FOUND CPPGSL_INCLUDE_DIR)

# Modernisation: create an interface target to link against
if(TARGET cppgsl::cppgsl)
    return()
endif()
if(CPPGSL_FOUND)
  add_library(cppgsl::cppgsl IMPORTED INTERFACE)
  target_include_directories(cppgsl::cppgsl SYSTEM INTERFACE "${CPPGSL_INCLUDE_DIR}")
  # Display the imported target for the user to know
  if(NOT ${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
    message(STATUS "  Import target: cppgsl::cppgsl")
  endif()
endif()

if(COMMAND __deprecate_var_for_target)
  foreach(v IN ITEMS CPPGSL_INCLUDE_DIR CPPGSL_INCLUDE_DIRS)
    variable_watch(${v} __deprecate_var_for_target)
  endforeach()
endif()

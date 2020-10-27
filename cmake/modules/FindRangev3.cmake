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
# Module locating the rangev3 headers.
#
# Defines:
#  - RANGEV3_FOUND
#  - RANGEV3_INCLUDE_DIR
#  - RANGEV3_INCLUDE_DIRS (not cached)
#
# Imports:
#
#  Rangev3::rangev3
#
# Usage of the target instead of the variables is advised

# Find quietly if already found before
if(DEFINED CACHE{RANGEV3_INCLUDE_DIR})
  set(${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY YES)
endif()

# Look for the header directory:
find_path( RANGEV3_INCLUDE_DIR
   NAMES range/v3/all.hpp
   HINTS $ENV{RANGEV3_ROOT_DIR}/include ${RANGEV3_ROOT_DIR}/include )
set( RANGEV3_INCLUDE_DIRS ${RANGEV3_INCLUDE_DIR} )

# Handle the regular find_package arguments:
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( Rangev3 DEFAULT_MSG RANGEV3_INCLUDE_DIR )

# Mark the cached variables as "advanced":
mark_as_advanced( RANGEV3_FOUND RANGEV3_INCLUDE_DIR )

# Modernisation: create an interface target to link against
if(TARGET Rangev3::rangev3)
    return()
endif()
if(RANGEV3_FOUND)
  add_library(Rangev3::rangev3 IMPORTED INTERFACE)
  target_include_directories(Rangev3::rangev3 SYSTEM INTERFACE "${RANGEV3_INCLUDE_DIRS}")
  # Display the imported target for the user to know
  if(NOT ${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
    message(STATUS "  Import target: Rangev3::rangev3")
  endif()
endif()

if(COMMAND __deprecate_var_for_target)
  foreach(v IN ITEMS RANGEV3_INCLUDE_DIR RANGEV3_INCLUDE_DIRS)
    variable_watch(${v} __deprecate_var_for_target)
  endforeach()
endif()

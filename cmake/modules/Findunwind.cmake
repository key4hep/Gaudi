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
# - Locate unwind library
# Defines:
#
#  UNWIND_FOUND
#  UNWIND_INCLUDE_DIR
#  UNWIND_INCLUDE_DIRS (not cached)
#  UNWIND_LIBRARIES
#  UNWIND_LIBRARY_DIRS (not cached)
#
# Imports:
#
#  unwind::unwind
#
# Usage of the target instead of the variables is advised

# Find quietly if already found before
if(DEFINED CACHE{UNWIND_INCLUDE_DIR})
  set(${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY YES)
endif()

find_path(UNWIND_INCLUDE_DIR unwind.h)
find_library(UNWIND_LIBRARIES NAMES unwind)

set(UNWIND_INCLUDE_DIRS ${UNWIND_INCLUDE_DIR})
get_filename_component(UNWIND_LIBRARY_DIRS ${UNWIND_LIBRARIES} PATH)

# handle the QUIETLY and REQUIRED arguments and set UNWIND_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(unwind DEFAULT_MSG UNWIND_INCLUDE_DIR UNWIND_LIBRARIES)

mark_as_advanced(UNWIND_FOUND UNWIND_INCLUDE_DIR UNWIND_LIBRARIES)

# Modernisation: create an interface target to link against
if(TARGET unwind::unwind)
    return()
endif()
if(UNWIND_FOUND)
  add_library(unwind::unwind IMPORTED INTERFACE)
  target_include_directories(unwind::unwind SYSTEM INTERFACE "${UNWIND_INCLUDE_DIRS}")
  target_link_libraries(unwind::unwind INTERFACE "${UNWIND_LIBRARIES}")
  # Display the imported target for the user to know
  if(NOT ${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
    message(STATUS "  Import target: unwind::unwind")
  endif()
endif()

if(COMMAND __deprecate_var_for_target)
  foreach(v IN ITEMS UNWIND_INCLUDE_DIR UNWIND_INCLUDE_DIRS UNWIND_LIBRARIES UNWIND_LIBRARY_DIRS)
    variable_watch(${v} __deprecate_var_for_target)
  endforeach()
endif()

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
# - Try to find AIDA
# Defines:
#
#  AIDA_FOUND
#  AIDA_INCLUDE_DIRS
#
# Imports:
#
#  AIDA::aida
#
# Usage of the target instead of the variables is advised

# Find quietly if already found before
if(DEFINED CACHE{AIDA_INCLUDE_DIRS})
  set(${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY YES)
endif()

find_path(AIDA_INCLUDE_DIRS AIDA/AIDA.h
          HINTS ${AIDA_ROOT_DIR} $ENV{AIDA_ROOT_DIR}
          PATH_SUFFIXES src/cpp ../share/src/cpp)

# handle the QUIETLY and REQUIRED arguments and set AIDA_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(AIDA DEFAULT_MSG AIDA_INCLUDE_DIRS)

mark_as_advanced(AIDA_INCLUDE_DIRS)

# Modernisation: create an interface target to link against
if(TARGET AIDA::aida)
    return()
endif()
if(AIDA_FOUND)
  add_library(AIDA::aida IMPORTED INTERFACE)
  target_include_directories(AIDA::aida SYSTEM INTERFACE "${AIDA_INCLUDE_DIRS}")
  # Display the imported target for the user to know
  if(NOT ${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
    message(STATUS "  Import target: AIDA::aida")
  endif()
endif()

if(COMMAND __deprecate_var_for_target)
  variable_watch(AIDA_INCLUDE_DIRS __deprecate_var_for_target)
endif()

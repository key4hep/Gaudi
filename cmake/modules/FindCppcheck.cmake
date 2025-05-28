#####################################################################################
# (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# - Locate cppcheck executable
# Defines:
#
#  CPPCHECK_FOUND
#  CPPCHECK_EXECUTABLE
#  CPPCHECK_VERSION
#

# Find quietly if already found before
if(DEFINED CACHE{CPPCHECK_EXECUTABLE})
  set(${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY YES)
endif()

find_program(CPPCHECK_EXECUTABLE NAMES cppcheck)

if( CPPCHECK_EXECUTABLE )
   execute_process( COMMAND "${CPPCHECK_EXECUTABLE}" --version
      OUTPUT_VARIABLE CPPCHECK_VERSION_OUTPUT
      OUTPUT_STRIP_TRAILING_WHITESPACE )
   string( REGEX REPLACE "^Cppcheck (.*)" "\\1"
      CPPCHECK_VERSION "${CPPCHECK_VERSION_OUTPUT}" )
   unset( CPPCHECK_VERSION_OUTPUT )
endif()

# Handle the standard find_package arguments:
include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( Cppcheck
   FOUND_VAR CPPCHECK_FOUND
   REQUIRED_VARS CPPCHECK_EXECUTABLE
   VERSION_VAR CPPCHECK_VERSION )

mark_as_advanced(CPPCHECK_FOUND CPPCHECK_EXECUTABLE CPPCHECK_VERSION)

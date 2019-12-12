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

find_path(UNWIND_INCLUDE_DIR unwind.h)
find_library(UNWIND_LIBRARIES NAMES unwind)

set(UNWIND_INCLUDE_DIRS ${UNWIND_INCLUDE_DIR})
get_filename_component(UNWIND_LIBRARY_DIRS ${UNWIND_LIBRARIES} PATH)

# handle the QUIETLY and REQUIRED arguments and set UNWIND_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(unwind DEFAULT_MSG UNWIND_INCLUDE_DIR UNWIND_LIBRARIES)

mark_as_advanced(UNWIND_FOUND UNWIND_INCLUDE_DIR UNWIND_LIBRARIES)

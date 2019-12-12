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
# - Locate UUID library
# Defines:
#
#  UUID_FOUND
#  UUID_INCLUDE_DIR
#  UUID_INCLUDE_DIRS (not cached)
#  UUID_LIBRARIES

find_path(UUID_INCLUDE_DIR uuid/uuid.h
          HINTS $ENV{UUID_ROOT_DIR}/include ${UUID_ROOT_DIR}/include)
find_library(UUID_LIBRARIES NAMES uuid System
          HINTS $ENV{UUID_ROOT_DIR}/lib ${UUID_ROOT_DIR}/lib)

set(UUID_INCLUDE_DIRS ${UUID_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set UUID_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(UUID DEFAULT_MSG UUID_INCLUDE_DIR UUID_LIBRARIES)

mark_as_advanced(UUID_FOUND UUID_INCLUDE_DIR UUID_LIBRARIES)

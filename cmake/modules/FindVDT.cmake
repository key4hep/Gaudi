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
# - Locate VDT math library
# Defines:
#
#  VDT_FOUND
#  VDT_INCLUDE_DIR
#  VDT_INCLUDE_DIRS (not cached)
#  VDT_LIBRARIES

find_path(VDT_INCLUDE_DIR vdt/vdtcore_common.h
          HINTS $ENV{VDT_ROOT_DIR}/include ${VDT_ROOT_DIR}/include)
find_library(VDT_LIBRARIES NAMES vdt
          HINTS $ENV{VDT_ROOT_DIR}/lib ${VDT_ROOT_DIR}/lib)

set(VDT_INCLUDE_DIRS ${VDT_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set VDT_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(VDT DEFAULT_MSG VDT_INCLUDE_DIR VDT_LIBRARIES)

mark_as_advanced(VDT_FOUND VDT_INCLUDE_DIR VDT_LIBRARIES)

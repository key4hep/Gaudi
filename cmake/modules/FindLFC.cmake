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
# - Locate LFC library
# Defines:
#
#  LFC_FOUND
#  LFC_INCLUDE_DIR
#  LFC_INCLUDE_DIRS (not cached)
#  LFC_LIBRARIES
#  LFC_LIBRARY_DIRS (not cached)

find_path(LFC_INCLUDE_DIR lfc/lfc_api.h)
find_library(LFC_LIBRARIES NAMES lfc PATH_SUFFIXES lib64)

set(LFC_INCLUDE_DIRS ${LFC_INCLUDE_DIR})
get_filename_component(LFC_LIBRARY_DIRS ${LFC_LIBRARIES} PATH)

# handle the QUIETLY and REQUIRED arguments and set LFC_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LFC DEFAULT_MSG LFC_INCLUDE_DIR LFC_LIBRARIES)

mark_as_advanced(LFC_FOUND LFC_INCLUDE_DIR LFC_LIBRARIES)

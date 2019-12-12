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
# - Locate XercesC library
# Defines:
#
#  XERCESC_FOUND
#  XERCESC_INCLUDE_DIR
#  XERCESC_INCLUDE_DIRS (not cached)
#  XERCESC_LIBRARY
#  XERCESC_LIBRARIES (not cached)
#  XERCESC_LIBRARY_DIRS (not cached)

find_path(XERCESC_INCLUDE_DIR xercesc/util/XercesVersion.hpp
          HINTS $ENV{XERCESC_ROOT_DIR}/include  ${XERCESC_ROOT_DIR}/include )
find_library(XERCESC_LIBRARY NAMES xerces-c
             HINTS $ENV{XERCESC_ROOT_DIR}/lib ${XERCESC_ROOT_DIR}/lib )

set(XERCESC_INCLUDE_DIRS ${XERCESC_INCLUDE_DIR})

# handle the QUIETLY and REQUIRED arguments and set XERCESC_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(XercesC DEFAULT_MSG XERCESC_INCLUDE_DIR XERCESC_LIBRARY)

mark_as_advanced(XERCESC_FOUND XERCESC_INCLUDE_DIR XERCESC_LIBRARY)

set(XERCESC_LIBRARIES ${XERCESC_LIBRARY})
set(XERCESC_LIBRARY_DIRS)
foreach(_lib ${XERCESC_LIBRARIES})
  get_filename_component(_libdir ${_lib} PATH)
  list(APPEND XERCESC_LIBRARY_DIRS ${_libdir})
endforeach()
if(XERCESC_LIBRARY_DIRS)
  list(REMOVE_DUPLICATES XERCESC_LIBRARY_DIRS)
endif()

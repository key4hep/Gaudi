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
# - Locate Xrootd library
# Defines:
#
#  XROOTD_FOUND
#  XROOTD_INCLUDE_DIR
#  XROOTD_INCLUDE_DIRS (not cached)
#  XROOTD_<component>_LIBRARY
#  XROOTD_<component>_FOUND
#  XROOTD_LIBRARIES (not cached)
#  XROOTD_LIBRARY_DIRS (not cached)
#  XROOTD_EXECUTABLE


# Enforce a minimal list if none is explicitly requested
if(NOT Xrootd_FIND_COMPONENTS)
  set(Xrootd_FIND_COMPONENTS Utils)
endif()

find_path(XROOTD_INCLUDE_DIR xrootd/XrdVersion.hh)
set(XROOTD_INCLUDE_DIRS ${XROOTD_INCLUDE_DIR})

set(XROOTD_LIBRARY_DIRS)

foreach(component ${Xrootd_FIND_COMPONENTS})
  find_library(XROOTD_${component}_LIBRARY NAMES Xrd${component} PATH_SUFFIXES lib64)
  if (XROOTD_${component}_LIBRARY)
    set(XROOTD_${component}_FOUND 1)
    list(APPEND XROOTD_LIBRARIES ${XROOTD_${component}_LIBRARY})

    get_filename_component(libdir ${XROOTD_${component}_LIBRARY} PATH)
    list(APPEND XROOTD_LIBRARY_DIRS ${libdir})
  else()
    set(XROOTD_${component}_FOUND 0)
  endif()
  mark_as_advanced(XROOTD_${component}_LIBRARY)
endforeach()

if(XROOTD_LIBRARY_DIRS)
  list(REMOVE_DUPLICATES XROOTD_LIBRARY_DIRS)
endif()

find_program(XROOTD_EXECUTABLE NAMES xrd
             HINTS ${XROOTD_INCLUDE_DIR}/../bin)

# handle the QUIETLY and REQUIRED arguments and set SQLITE_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Xrootd DEFAULT_MSG XROOTD_INCLUDE_DIR XROOTD_LIBRARIES)

mark_as_advanced(XROOTD_FOUND XROOTD_INCLUDE_DIR XROOTD_EXECUTABLE)

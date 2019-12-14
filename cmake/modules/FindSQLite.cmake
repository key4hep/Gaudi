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
# - Locate SQLite library
# Defines:
#
#  SQLITE_FOUND
#  SQLITE_INCLUDE_DIR
#  SQLITE_INCLUDE_DIRS (not cached)
#  SQLITE_LIBRARY
#  SQLITE_LIBRARIES (not cached)
#  SQLITE_LIBRARY_DIRS (not cached)
#  SQLITE_EXECUTABLE

find_path(SQLITE_INCLUDE_DIR sqlite3.h)
find_library(SQLITE_LIBRARY NAMES sqlite3)

find_program(SQLITE_EXECUTABLE NAMES sqlite3
             HINTS ${SQLITE_INCLUDE_DIR}/../bin)

# handle the QUIETLY and REQUIRED arguments and set SQLITE_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SQLite DEFAULT_MSG SQLITE_INCLUDE_DIR SQLITE_LIBRARY)

mark_as_advanced(SQLITE_FOUND SQLITE_INCLUDE_DIR SQLITE_LIBRARY SQLITE_EXECUTABLE)

if(SQLITE_LIBRARY)
  get_filename_component(SQLITE_LIBRARY_DIRS ${SQLITE_LIBRARY} PATH)
  set(SQLITE_LIBRARIES ${SQLITE_LIBRARY})
endif()

set(SQLITE_INCLUDE_DIRS ${SQLITE_INCLUDE_DIR})

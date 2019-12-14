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
# - Locate MySQL library
# Defines:
#
#  MYSQL_FOUND
#  MYSQL_INCLUDE_DIR
#  MYSQL_INCLUDE_DIRS (not cached)
#  MYSQL_LIBRARY
#  MYSQL_LIBRARIES (not cached)
#  MYSQL_LIBRARY_DIRS (not cached)
#  MYSQL_EXECUTABLE

find_path(MYSQL_INCLUDE_DIR mysql.h)
find_library(MYSQL_LIBRARY NAMES mysqlclient_r libmysql.lib)

# handle the QUIETLY and REQUIRED arguments and set MYSQL_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MySQL DEFAULT_MSG MYSQL_INCLUDE_DIR MYSQL_LIBRARY)

mark_as_advanced(MYSQL_FOUND MYSQL_INCLUDE_DIR MYSQL_LIBRARY)

set(MYSQL_INCLUDE_DIRS ${MYSQL_INCLUDE_DIR} ${MYSQL_INCLUDE_DIR}/mysql)
get_filename_component(MYSQL_LIBRARY_DIRS ${MYSQL_LIBRARY} PATH)

set(MYSQL_LIBRARIES ${MYSQL_LIBRARY})

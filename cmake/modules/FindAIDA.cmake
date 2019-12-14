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

find_path(AIDA_INCLUDE_DIRS AIDA/AIDA.h
          HINTS ${AIDA_ROOT_DIR} $ENV{AIDA_ROOT_DIR}
          PATH_SUFFIXES src/cpp ../share/src/cpp)

# handle the QUIETLY and REQUIRED arguments and set AIDA_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(AIDA DEFAULT_MSG AIDA_INCLUDE_DIRS)

mark_as_advanced(AIDA_INCLUDE_DIRS)

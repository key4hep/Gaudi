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
# Gaudi helper cpack script
message("CPACK Install Script")
set(CPACK_SET_DESTDIR FALSE)
set(ENV{CMAKE_LINK_PREFIX} "${CPACK_TEMPORARY_DIRECTORY}/")
if(CPACK_SOURCE_PACKAGE_FILE_NAME)
  message(STATUS "Setting CPACK_INSTALLED_DIRECTORIES to CPACK_SOURCE_INSTALLED_DIRECTORIES")
  set(CPACK_INSTALLED_DIRECTORIES "${CPACK_SOURCE_INSTALLED_DIRECTORIES}")
  set(CPACK_RPM_PACKAGE_ARCHITECTURE "${CPACK_SOURCE_RPM_PACKAGE_ARCHITECTURE}")
  set(CPACK_RPM_PACKAGE_NAME "${CPACK_SOURCE_RPM_PACKAGE_NAME}")
endif()

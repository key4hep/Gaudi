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
#--- CPack configuration
set(CPACK_PACKAGE_NAME ${project})
foreach(t MAJOR MINOR PATCH)
   set(CPACK_PACKAGE_VERSION_${t} ${CMAKE_PROJECT_VERSION_${t}})
endforeach()
set(CPACK_SYSTEM_NAME ${BINARY_TAG})

set(CPACK_SOURCE_IGNORE_FILES "/InstallArea/;/build\\\\..*/;/\\\\.svn/;/\\\\.git/;/\\\\.settings/;\\\\..*project;\\\\.gitignore")

# for the binary
set(CPACK_INSTALL_PREFIX "usr/${project}/${version}/InstallArea/${BINARY_TAG}")
set(CPACK_INSTALL_SCRIPT "${GaudiProject_DIR}/cpack_install.cmake")

# for the source
set(CPACK_SOURCE_INSTALLED_DIRECTORIES "${CMAKE_SOURCE_DIR};/usr/${project}/${version}")

# for the RPMs
set(CPACK_PACKAGE_DEFAULT_LOCATION "/usr")
set(CPACK_GENERATOR "RPM")
set(CPACK_RPM_PACKAGE_VERSION "${version}")
set(CPACK_SOURCE_GENERATOR "RPM")
set(CPACK_SOURCE_RPM "ON")
set(CPACK_SOURCE_RPM_PACKAGE_ARCHITECTURE "noarch")
set(CPACK_SOURCE_RPM_PACKAGE_NAME "${project}-source")

include(CPack)

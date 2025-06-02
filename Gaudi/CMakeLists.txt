#####################################################################################
# (c) Copyright 1998-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# Gaudi subdirectory

# Build binaries
gaudi_add_executable(Gaudi
                     SOURCES src/main.cpp
                     LINK GaudiKernel)
# Add a symlink Gaudi.exe -> Gaudi because some tests expect to use Gaudi.exe
file(CREATE_LINK Gaudi ${CMAKE_CURRENT_BINARY_DIR}/Gaudi.exe SYMBOLIC)

# Generate Gaudi_user.confdb
gaudi_generate_confuserdb(Gaudi.ConfUser)

# Python tests
gaudi_add_pytest(tests/pytest)

# Record project version in a Python module
set(GIT_DESCRIBE_TAG "unknown")
if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
    find_package(Git)
    if(GIT_FOUND)
        execute_process(
          COMMAND ${GIT_EXECUTABLE} describe --tags
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
          OUTPUT_VARIABLE GIT_DESCRIBE_TAG
          OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()
endif()

# set optional version tweak and patch
if(NOT PROJECT_VERSION_TWEAK)
    set(PROJECT_VERSION_TWEAK 0)
endif()
if(NOT PROJECT_VERSION_PATCH)
    set(PROJECT_VERSION_PATCH 0)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/_version.py.in
               ${CMAKE_BINARY_DIR}/python/Gaudi/_version.py @ONLY)

# Install python modules
gaudi_install(PYTHON)
install(FILES ${CMAKE_BINARY_DIR}/python/Gaudi/_version.py
        DESTINATION "${GAUDI_INSTALL_PYTHONDIR}/Gaudi")

# Install other scripts
gaudi_install(SCRIPTS)

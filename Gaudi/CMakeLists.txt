#####################################################################################
# (c) Copyright 1998-2021 CERN for the benefit of the LHCb and ATLAS collaborations #
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

# QMTest
gaudi_add_tests(QMTest)

# Python tests
gaudi_add_pytest(tests/nose)
gaudi_add_pytest(python/Gaudi/Main.py
    OPTIONS --doctest-modules
    ROOT_DIR python
    PREFIX Gaudi.doctest.python.
)

# Install python modules
gaudi_install(PYTHON)
# Install other scripts
gaudi_install(SCRIPTS)


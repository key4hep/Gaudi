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

# nosetests
gaudi_add_tests(nosetests)
gaudi_add_tests(nosetests ${CMAKE_CURRENT_SOURCE_DIR}/python/Gaudi)

# Install python modules
gaudi_install(PYTHON)
# Install other scripts
gaudi_install(SCRIPTS)

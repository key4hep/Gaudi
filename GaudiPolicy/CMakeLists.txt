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
# GaudiPolicy subdirectory

# Install python modules
gaudi_install(PYTHON)
# Install other scripts
gaudi_install(SCRIPTS)

if(BUILD_TESTING)
    # QMTest
    # GAUDI-976: make GaudiTest.py more resilient towards unicode-ascii conversion failure
    # https://its.cern.ch/jira/browse/GAUDI-976
    gaudi_add_tests(QMTest ${CMAKE_CURRENT_SOURCE_DIR}/tests/GAUDI-976)

    # nosetests
    gaudi_add_tests(nosetests ${CMAKE_CURRENT_SOURCE_DIR}/python/GaudiTesting)
endif()

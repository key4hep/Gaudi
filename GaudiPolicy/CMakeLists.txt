#####################################################################################
# (c) Copyright 1998-2024 CERN for the benefit of the LHCb and ATLAS collaborations #
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
    gaudi_add_tests(QMTest)

    # GAUDI-976: make GaudiTest.py more resilient towards unicode-ascii conversion failure
    # We inject unicode in the environment to make sure it's handles correctly
    # https://its.cern.ch/jira/browse/GAUDI-976
    set_tests_properties(GaudiPolicy.jira_gaudi_976
        PROPERTIES
            ENVIRONMENT TESTENV=à
    )

    # the default stdout limit is 100MB, but for to test the check we can use a smaller limit (1KB)
    set_tests_properties(GaudiPolicy.stdout_cutoff
        PROPERTIES
            ENVIRONMENT GAUDI_TEST_STDOUT_LIMIT=1024
            PASS_REGULAR_EXPRESSION "unexpected too big stdout"
    )

    gaudi_add_tests(pytest ${CMAKE_CURRENT_SOURCE_DIR}/python/GaudiTesting)
    gaudi_add_pytest(tests/pytest)
endif()

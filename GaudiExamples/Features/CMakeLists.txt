#####################################################################################
# (c) Copyright 2024-2025 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################
# Add tests to check that we can run the examples.

set(prefix GaudiExamples.Features)

add_test(
    NAME ${prefix}.event_timeout_check
    COMMAND ${CMAKE_COMMAND} -E env $<TARGET_FILE:run> gaudirun.py ${CMAKE_CURRENT_SOURCE_DIR}/event_timeout_check.py:config
)
set_tests_properties(
    ${prefix}.event_timeout_check
    PROPERTIES
        LABELS "GaudiExamples;GaudiExamples.Features"
        PASS_REGULAR_EXPRESSION "More than 2s since the beginning of the event.*aborting process"
        TIMEOUT 120
)

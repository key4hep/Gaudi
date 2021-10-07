#!/usr/bin/env gaudirun.py
#####################################################################################
# (c) Copyright 2021 CERN for the benefit of the LHCb and ATLAS collaborations #
#                                                                                   #
# This software is distributed under the terms of the Apache version 2 licence,     #
# copied verbatim in the file "LICENSE".                                            #
#                                                                                   #
# In applying this licence, CERN does not waive the privileges and immunities       #
# granted to it by virtue of its status as an Intergovernmental Organization        #
# or submit itself to any jurisdiction.                                             #
#####################################################################################

# import special return code to indicate that the test is skipped
from GaudiTesting import SKIP_RETURN_CODE
import sys

# NOTE: returning SKIP_RETURN_CODE is meant for tests which should be skipped
#       based on runtime information.
#       If you know at configuration time that a tests should be skipped
#       perfer disabling the test from CMake by setting the DISABLED propertey:
#       set_property(
#         TEST
#             the_name_of_your_test
#         APPEND PROPERTY
#         DISABLED TRUE
#        )

# exit and return the code indicating that this test was skipped
sys.exit(SKIP_RETURN_CODE)

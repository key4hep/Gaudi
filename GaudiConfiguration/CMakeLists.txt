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
# GaudiConfiguration subdirectory

# Install python modules
gaudi_install(PYTHON)

gaudi_add_pytest(tests/python
    COVERAGE GaudiConfig2 COVERAGE_OPTIONS --fail-under=99 --show-missing
)

# Note: The file tools/print_limits.cpp is a ROOT macro, so it is not meant to be compiled

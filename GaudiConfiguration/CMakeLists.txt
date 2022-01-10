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
# GaudiConfiguration subdirectory

# Install python modules
gaudi_install(PYTHON)

if(BUILD_TESTING)
  # FIXME: gaudi_add_test(nosetests) is not configurable enough, so I copied a bit of it
  _import_nosetests()
  add_test(NAME GaudiConfiguration.nose
           COMMAND run $<TARGET_FILE:nosetests> -v --with-doctest --with-coverage --cover-package=GaudiConfig2
                   --cover-min-percentage=100
                   ${CMAKE_CURRENT_SOURCE_DIR}/tests/nose)
endif()

# Note: The file tools/print_limits.cpp is a ROOT macro, so it is not meant to be compiled

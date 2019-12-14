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
gaudi_subdir(GaudiConfiguration)


gaudi_install_python_modules()

# Ideally I would use '--cover-min-percentage=100', but the version of nose we
# have is a bit old
gaudi_add_test(nose
               COMMAND nosetests -v --with-doctest --with-coverage --cover-package=GaudiConfig2
                 ${CMAKE_CURRENT_SOURCE_DIR}/tests/nose
               PASSREGEX "TOTAL .* 100%"
               FAILREGEX "FAILED")

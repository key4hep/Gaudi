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
gaudi_subdir(GaudiPolicy)

#---Installation-----------------------------------------------------------------------------
gaudi_install_scripts()
gaudi_install_python_modules()

find_program(nosetests_CMD NAMES nosetests)
if (nosetests_CMD AND NOT nosetests_CMD MATCHES "^(/usr(/local)?)?/bin/")
  get_filename_component(nosetests_DIR "${nosetests_CMD}" DIRECTORY)
  gaudi_env(PREPEND PATH "${nosetests_DIR}")
endif()

# Set environment to access the custom QMTest classes (at build time).
gaudi_build_env(PREPEND QMTEST_CLASS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/qmtest_classes)

# Install QMTest extensions.
install(DIRECTORY qmtest_classes DESTINATION .)
# Note: ${.} in the environment definition refers to the install prefix.
gaudi_env(PREPEND QMTEST_CLASS_PATH \${.}/qmtest_classes)

# Skeleton required by the script cTestXml2HTML.py
install(DIRECTORY data/HTMLTestReportSkel DESTINATION data)

# Search path for data files
gaudi_build_env(PREPEND DATA_PATH ${CMAKE_CURRENT_SOURCE_DIR}/data)
# Note: ${.} in the environment definition refers to the install prefix.
gaudi_env(PREPEND DATA_PATH \${.}/data)

# -- Tests --
if(GAUDI_BUILD_TESTS)
  # GAUDI-976: make GaudiTest.py more resilient towards unicode-ascii conversion failure
  # https://its.cern.ch/jira/browse/GAUDI-976
  configure_file(tests/GAUDI-976/test.qmt GAUDI-976/test.qmt COPYONLY)
  configure_file(tests/GAUDI-976/QMTest/configuration GAUDI-976/QMTest/configuration COPYONLY)
  # GAUDI-976 can be written as qmtest directly
  gaudi_add_test(GAUDI-976 QMTEST
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/GAUDI-976
    ENVIRONMENT TESTENV=à
    PASSREGEX "1 \\(100%\\) tests PASS")
endif()

gaudi_add_test(GaudiTesting.nose
               COMMAND nosetests --with-doctest
                 ${CMAKE_CURRENT_SOURCE_DIR}/python/GaudiTesting)

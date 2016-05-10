gaudi_subdir(GaudiPolicy v15r8)

#---Installation-----------------------------------------------------------------------------
gaudi_install_scripts()
gaudi_install_python_modules()

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
  find_package(QMTest)
  gaudi_add_test(GAUDI-976
                 COMMAND ${env_cmd} --xml ${env_xml} TESTENV=à qmtest run
                 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/GAUDI-976
                 PASSREGEX "1 \\(100%\\) tests PASS")
endif()

gaudi_add_test(GaudiTesting.nose
               COMMAND nosetests --with-doctest
                 ${CMAKE_CURRENT_SOURCE_DIR}/python/GaudiTesting)

gaudi_add_test(GAUDI-1105
               COMMAND ./test_install.py
               WORKING_DIRECTORY
               ${CMAKE_CURRENT_SOURCE_DIR}/tests/GAUDI-1105)

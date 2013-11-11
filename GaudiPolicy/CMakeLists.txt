gaudi_subdir(GaudiPolicy v12r8)

#---Installation-----------------------------------------------------------------------------
gaudi_install_scripts()

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

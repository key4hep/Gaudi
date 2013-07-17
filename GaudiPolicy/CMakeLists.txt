gaudi_subdir(GaudiPolicy v12r7)

#---Installation-----------------------------------------------------------------------------
gaudi_install_scripts()

# Set environment to access the custom QMTest classes (at build time).
gaudi_build_env(PREPEND QMTEST_CLASS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/qmtest_classes)

# Install QMTest extensions.
install(DIRECTORY qmtest_classes DESTINATION .)
# Note: ${.} in the environment definition refers to the install prefix.
gaudi_env(PREPEND QMTEST_CLASS_PATH \${.}/qmtest_classes)

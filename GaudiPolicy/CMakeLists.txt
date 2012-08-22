gaudi_subdir(GaudiPolicy v12r1)

#---Installation-----------------------------------------------------------------------------
gaudi_install_scripts()

# Set environment to access the custom QMTest classes.
gaudi_env(PREPEND QMTEST_CLASS_PATH \${GAUDIPOLICYROOT}/qmtest_classes)

# FIXME: the QMTest extensions must be installed and not used from the sources.
# Install QMTest extensions.
#install(DIRECTORY qmtest_classes DESTINATION .)

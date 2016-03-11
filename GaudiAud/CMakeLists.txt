gaudi_subdir(GaudiAud v10r5)

gaudi_depends_on_subdirs(GaudiKernel)

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiAud src/*.cpp LINK_LIBRARIES GaudiKernel)

gaudi_add_test(QMTest QMTEST)

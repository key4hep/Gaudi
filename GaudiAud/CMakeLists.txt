gaudi_subdir(GaudiAud v9r6)

gaudi_depends_on_subdirs(GaudiKernel)

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiAud *.cpp LINK_LIBRARIES GaudiKernel)

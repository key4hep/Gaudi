gaudi_subdir(GaudiAud v9r9)

gaudi_depends_on_subdirs(GaudiKernel)

#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiAud src/*.cpp LINK_LIBRARIES GaudiKernel)

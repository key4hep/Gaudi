gaudi_subdir(GaudiGPU v1r0)

gaudi_depends_on_subdirs(GaudiKernel GaudiAlg)


#---Libraries---------------------------------------------------------------
gaudi_add_module(GaudiGPU
                 *.cpp
                 LINK_LIBRARIES GaudiKernel GaudiAlgLib)
gaudi_subdir(GaudiPartProp v1r2)

gaudi_depends_on_subdirs(GaudiKernel)

gaudi_add_module(GaudiPartProp src/ParticlePropertySvc.cpp
                 LINK_LIBRARIES GaudiKernel)

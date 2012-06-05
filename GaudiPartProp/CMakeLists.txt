gaudi_subdir(GaudiPartProp v1r0)

gaudi_depends_on_subdirs(GaudiKernel)

gaudi_add_module(GaudiPartProp src/ParticlePropertySvc.cpp
                        LINK_LIBRARIES GaudiKernel)

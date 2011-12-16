require(GaudiKernel)

gaudi_component_library(GaudiPartProp src/ParticlePropertySvc.cpp
                        LIBRARIES GaudiKernel)

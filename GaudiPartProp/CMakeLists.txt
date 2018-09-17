gaudi_subdir(GaudiPartProp)

gaudi_depends_on_subdirs(GaudiKernel)

find_package(Boost REQUIRED)

# Hide some Boost compile time warnings
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

gaudi_add_module(GaudiPartProp src/ParticlePropertySvc.cpp
                 LINK_LIBRARIES GaudiKernel)

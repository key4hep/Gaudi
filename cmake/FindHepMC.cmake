INCLUDE(Configuration)

SET(HepMC_native_version ${HepMC_config_version})
SET(HepMC_home ${LCG_external}/HepMC/${HepMC_native_version}/${LCG_system})

SET(HepMC_FOUND 1)
SET(HepMC_INCLUDE_DIRS ${HepMC_home}/include)
SET(HepMC_LIBRARY_DIRS ${HepMC_home}/lib)
SET(HepMC_LIBRARIES HepMC)
 
SET(HepMC_environment LD_LIBRARY_PATH+=${HepMC_home}/lib)


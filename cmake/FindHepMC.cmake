include(Configuration)

set(HepMC_native_version ${HepMC_config_version})
set(HepMC_home ${LCG_external}/HepMC/${HepMC_native_version}/${LCG_system})

set(HepMC_FOUND 1)
set(HepMC_INCLUDE_DIRS ${HepMC_home}/include)
set(HepMC_LIBRARY_DIRS ${HepMC_home}/lib)
set(HepMC_LIBRARIES HepMC)
 
if(WIN32)
  set(HepMC_environment PATH+=${HepMC_home}/lib)
else()
  set(HepMC_environment LD_LIBRARY_PATH+=${HepMC_home}/lib)
endif()


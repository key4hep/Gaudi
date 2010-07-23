INCLUDE(Configuration)

SET(HepPDT_native_version ${HepPDT_config_version})
SET(HepPDT_home ${LCG_external}/HepPDT/${HepPDT_native_version}/${LCG_system})

SET(HepPDT_FOUND 1)
SET(HepPDT_INCLUDE_DIRS ${HepPDT_home}/include)
SET(HepPDT_LIBRARY_DIRS ${HepPDT_home}/lib)
SET(HepPDT_LIBRARIES HepPDT HepPID)
 
SET(HepPDT_environment LD_LIBRARY_PATH+=${HepPDT_home}/lib)


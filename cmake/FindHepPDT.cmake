include(Configuration)

set(HepPDT_native_version ${HepPDT_config_version})
set(HepPDT_home ${LCG_external}/HepPDT/${HepPDT_native_version}/${LCG_system})

set(HepPDT_FOUND 1)
set(HepPDT_INCLUDE_DIRS ${HepPDT_home}/include)
set(HepPDT_LIBRARY_DIRS ${HepPDT_home}/lib)
set(HepPDT_LIBRARIES HepPDT HepPID)
 
if(WIN32)
  set(HepPDT_environment PATH+=${HepPDT_home}/lib)
else()
  set(HepPDT_environment LD_LIBRARY_PATH+=${HepPDT_home}/lib)
endif()

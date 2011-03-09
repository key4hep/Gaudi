include(Configuration)

set(Python_native_version ${Python_config_version})
set(Python_home ${LCG_external}/Python/${Python_native_version}/${LCG_system})

if(WIN32)
  set(Python_environment PATH+=${Python_home}/bin)
else()
  set(Python_environment LD_LIBRARY_PATH+=${Python_home}/lib
                         PATH+=${Python_home}/bin)
endif()

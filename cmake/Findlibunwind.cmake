include(Configuration)

set(libunwind_native_version ${libunwind_config_version})
set(libunwind_home ${LCG_external}/libunwind/${libunwind_native_version}/${LCG_system})

set(libunwind_FOUND 1)
set(libunwind_INCLUDE_DIRS ${libunwind_home}/include)
set(libunwind_LIBRARY_DIRS ${libunwind_home}/lib)
set(libunwind_LIBRARIES libunwind )

if(WIN32)
  set(libunwind_environment PATH+=${libunwind_home}/lib)
else()
  SET(libunwind_environment LD_LIBRARY_PATH+=${libunwind_home}/lib)
endif()

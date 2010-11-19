include(Configuration)

set(uuid_native_version ${uuid_config_version})
set(uuid_home ${LCG_external}/uuid/${uuid_native_version}/${LCG_system})
set(Boost_compiler_version gcc43)


set(uuid_FOUND 1)
set(uuid_INCLUDE_DIRS ${uuid_home}/include)
set(uuid_LIBRARY_DIRS ${uuid_home}/lib)
set(uuid_LIBRARIES uuid )

if(WIN32)
  set(uuid_environment PATH+=${uuid_home}/lib)
else()
  SET(uuid_environment LD_LIBRARY_PATH+=${uuid_home}/lib)
endif()
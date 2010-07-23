INCLUDE(Configuration)

SET(uuid_native_version ${uuid_config_version})
SET(uuid_home ${LCG_external}/uuid/${uuid_native_version}/${LCG_system})
SET(Boost_compiler_version gcc43)


SET(uuid_FOUND 1)
SET(uuid_INCLUDE_DIRS ${uuid_home}/include)
SET(uuid_LIBRARY_DIRS ${uuid_home}/lib)
SET(uuid_LIBRARIES uuid )

SET(uuid_environment LD_LIBRARY_PATH+=${uuid_home}/lib)

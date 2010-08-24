INCLUDE(Configuration)

SET(CppUnit_native_version ${CppUnit_config_version})
SET(CppUnit_home ${LCG_external}/CppUnit/${CppUnit_native_version}/${LCG_system})
SET(Boost_compiler_version gcc43)


SET(CppUnit_FOUND 1)
SET(CppUnit_INCLUDE_DIRS ${CppUnit_home}/include)
SET(CppUnit_LIBRARY_DIRS ${CppUnit_home}/lib)
SET(CppUnit_LIBRARIES cppunit )

SET(CppUnit_environment LD_LIBRARY_PATH+=${CppUnit_home}/lib)

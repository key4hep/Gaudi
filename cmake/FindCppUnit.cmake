include(Configuration)

set(CppUnit_native_version ${CppUnit_config_version})
set(CppUnit_home ${LCG_external}/CppUnit/${CppUnit_native_version}/${LCG_system})
set(Boost_compiler_version gcc43)


set(CppUnit_FOUND 1)
set(CppUnit_INCLUDE_DIRS ${CppUnit_home}/include)
set(CppUnit_LIBRARY_DIRS ${CppUnit_home}/lib)
set(CppUnit_LIBRARIES cppunit )

if(WIN32)
  set(CppUnit_environment PATH+=${CppUnit_home}/lib)
else()
  set(CppUnit_environment LD_LIBRARY_PATH+=${CppUnit_home}/lib)
endif()
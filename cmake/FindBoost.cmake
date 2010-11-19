include(Configuration)

set(Boost_native_version ${Boost_config_version}_python${Python_config_version_twodigit})
set(Boost_home ${LCG_external}/Boost/${Boost_native_version}/${LCG_system})

if(CMAKE_SYSTEM_NAME MATCHES Darwin)
  set(Boost_compiler_version xgcc42)
elseif (MSVC90)
  set(Boost_compiler_version vc90)
elseif (MSVC10)
  set(Boost_compiler_version vc100)
elseif (MSVC80)
  set(Boost_compiler_version vc80)
elseif (MSVC71)
  set(Boost_compiler_version vc71)
else()
  set(Boost_compiler_version gcc43)
endif()

set(Boost_COMPONENTS date_time filesystem graph iostreams math_c99 math_c99f math_tr1 math_tr1f
    prg_exec_monitor program_options python random regex serialization signals system thread 
	unit_test_framework wave wserialization)
	
foreach(component ${Boost_COMPONENTS})
  set(Boost_${component}_LIBRARY boost_${component}-${Boost_compiler_version}-mt )
endforeach()

set(Boost_FOUND 1)
set(Boost_VERSION ${Boost_config_version})

set(Boost_INCLUDE_DIRS ${Boost_home}/include/boost-${Boost_file_version})
set(Boost_LIBRARY_DIRS ${Boost_home}/lib)
set(Boost_LIBRARIES ${Boost_filesystem_LIBRARY} ${Boost_system_LIBRARY} ${Boost_thread_LIBRARY} ${Boost_program_options_LIBRARY} 
                    ${Boost_date_time_LIBRARY} ${Boost_regex_LIBRARY} )

if(WIN32)
  set(Boost_environment PATH+=${Boost_home}/lib)
else()
  set(Boost_environment LD_LIBRARY_PATH+=${Boost_home}/lib)
endif()
 
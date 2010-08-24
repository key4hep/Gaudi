INCLUDE(Configuration)

SET(Boost_native_version ${Boost_config_version}_python${Python_config_version_twodigit})
SET(Boost_home ${LCG_external}/Boost/${Boost_native_version}/${LCG_system})
IF (CMAKE_SYSTEM_NAME MATCHES Darwin)
  SET(Boost_compiler_version xgcc42)   
ELSE()
  SET(Boost_compiler_version gcc43)
ENDIF()

SET(Boost_filesystem boost_filesystem-${Boost_compiler_version}-mt )
SET(Boost_system boost_system-${Boost_compiler_version}-mt )
SET(Boost_thread boost_thread-${Boost_compiler_version}-mt )
SET(Boost_program_options boost_program_options-${Boost_compiler_version}-mt )
SET(Boost_regex boost_regex-${Boost_compiler_version}-mt )
SET(Boost_date_time boost_date_time-${Boost_compiler_version}-mt )

#SET(Boost_libraries ${Boost_filesystem} ${Boost_system} ${Boost_thread} ${Boost_program_options} )

SET(Boost_FOUND 1)
SET(Boost_INCLUDE_DIRS ${Boost_home}/include/boost-${Boost_file_version})
SET(Boost_LIBRARY_DIRS ${Boost_home}/lib)
SET(Boost_LIBRARIES ${Boost_filesystem} ${Boost_system} ${Boost_thread} ${Boost_program_options} ${Boost_regex} )

SET(Boost_environment LD_LIBRARY_PATH+=${Boost_home}/lib)

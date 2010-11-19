include(Configuration)


set(CORAL_native_version ${CORAL_config_version})
set(CORAL_base ${LCG_releases}/CORAL/${CORAL_native_version})
set(CORAL_home ${CORAL_base}/${LCG_platform})

set(CORAL_FOUND 1)
set(CORAL_INCLUDE_DIRS ${CORAL_base}/include)
set(CORAL_LIBRARY_DIRS ${CORAL_home}/lib)
set(CORAL_LIBRARIES lcg_CoralBase )

if(WIN32)
  SET(CORAL_environment PATH+=${CORAL_home}/lib)
else()
  SET(CORAL_environment LD_LIBRARY_PATH+=${CORAL_home}/lib)
endif()


include(Configuration)

set(COOL_native_version ${COOL_config_version})
set(COOL_base ${LCG_releases}/COOL/${COOL_native_version})
set(COOL_home ${COOL_base}/${LCG_platform})

set(COOL_FOUND 1)
set(COOL_INCLUDE_DIRS ${COOL_base}/include)
set(COOL_LIBRARY_DIRS ${COOL_home}/lib)
SET(COOL_LIBRARIES lcg_CoolKernel )

if(WIN32)
  SET(COOL_environment PATH+=${COOL_home}/lib)
else()
  SET(COOL_environment LD_LIBRARY_PATH+=${COOL_home}/lib)
endif()

